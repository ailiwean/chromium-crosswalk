/*	
    WebTextRenderer.m	    
    Copyright 2002, Apple, Inc. All rights reserved.
*/

#import "WebTextRenderer.h"

#import <Cocoa/Cocoa.h>

#import <ApplicationServices/ApplicationServices.h>
#import <CoreGraphics/CoreGraphicsPrivate.h>

#import <WebCore/WebCoreUnicode.h>

#import <WebKit/WebGlyphBuffer.h>
#import <WebKit/WebKitLogging.h>
#import <WebKit/WebTextRendererFactory.h>
#import <WebKit/WebUnicode.h>

#import <QD/ATSUnicodePriv.h>

#import <AppKit/NSFont_Private.h>

#import <float.h>

#define NON_BREAKING_SPACE 0x00A0
#define SPACE 0x0020

#define IS_CONTROL_CHARACTER(c) ((c) < 0x0020 || (c) == 0x007F)

#define ROUND_TO_INT(x) (unsigned int)((x)+.5)

// Lose precision beyond 1000ths place. This is to work around an apparent
// bug in CoreGraphics where there seem to be small errors to some metrics.
#define CEIL_TO_INT(x) ((int)(x + 0.999)) /* ((int)(x + 1.0 - FLT_EPSILON)) */

#define LOCAL_BUFFER_SIZE 1024

// Covers Latin1.
#define INITIAL_BLOCK_SIZE 0x200

// Get additional blocks of glyphs and widths in bigger chunks.
// This will typically be for other character sets.
#define INCREMENTAL_BLOCK_SIZE 0x400

#define UNINITIALIZED_GLYPH_WIDTH 65535

// combining char, hangul jamo, or Apple corporate variant tag
#define JunseongStart 0x1160
#define JonseongEnd 0x11F9
#define IsHangulConjoiningJamo(X) (X >= JunseongStart && X <= JonseongEnd)
#define IsNonBaseChar(X) ((CFCharacterSetIsCharacterMember(nonBaseChars, X) || IsHangulConjoiningJamo(X) || (((X) & 0x1FFFF0) == 0xF870)))

typedef float WebGlyphWidth;
typedef UInt32 UnicodeChar;

struct WidthEntry {
    WebGlyphWidth width;
};

struct WidthMap {
    ATSGlyphRef startRange;
    ATSGlyphRef endRange;
    WidthMap *next;
    WidthEntry *widths;
};

struct GlyphEntry
{
    ATSGlyphRef glyph;
    NSFont *font;
};

struct GlyphMap {
    UniChar startRange;
    UniChar endRange;
    GlyphMap *next;
    GlyphEntry *glyphs;
};

struct UnicodeGlyphMap {
    UnicodeChar startRange;
    UnicodeChar endRange;
    UnicodeGlyphMap *next;
    GlyphEntry *glyphs;
};

struct SubstituteFontWidthMap {
    NSFont *font;
    WidthMap *map;
};


@interface NSLanguage : NSObject 
{
}
+ (NSLanguage *)defaultLanguage;
@end

@interface NSFont (WebPrivate)
- (ATSUFontID)_atsFontID;
- (CGFontRef)_backingCGSFont;
// Private method to find a font for a character.
+ (NSFont *) findFontLike:(NSFont *)aFont forCharacter:(UInt32)c inLanguage:(NSLanguage *) language;
+ (NSFont *) findFontLike:(NSFont *)aFont forString:(NSString *)string withRange:(NSRange)range inLanguage:(NSLanguage *) language;
- (NSGlyph)_defaultGlyphForChar:(unichar)uu;
- (BOOL)_forceAscenderDelta;
- (BOOL)_canDrawOutsideLineHeight;
- (BOOL)_isSystemFont;
- (BOOL)_isFakeFixedPitch;
@end

@class NSCGSFont;

// FIXME.  This is a horrible workaround to the problem described in 3129490
@interface NSCGSFont : NSFont {
    NSFaceInfo *_faceInfo;
    id _otherFont;	// Try to get rid of this???
    float *_matrix;
    struct _NS_cgsResv *_reservedCGS;
    float _constWidth;		// if isFixedPitch, the actual scaled width
}
@end

static CFCharacterSetRef nonBaseChars = NULL;


@interface WebTextRenderer (WebPrivate)
- (WidthMap *)extendGlyphToWidthMapToInclude:(ATSGlyphRef)glyphID font:(NSFont *)font;
- (ATSGlyphRef)extendCharacterToGlyphMapToInclude:(UniChar) c;
- (ATSGlyphRef)extendUnicodeCharacterToGlyphMapToInclude: (UnicodeChar)c;
- (void)updateGlyphEntryForCharacter: (UniChar)c glyphID: (ATSGlyphRef)glyphID font: (NSFont *)substituteFont;
@end


static void freeWidthMap (WidthMap *map)
{
    if (!map)
	return;
    freeWidthMap (map->next);
    free (map->widths);
    free (map);
}


static void freeGlyphMap (GlyphMap *map)
{
    if (!map)
	return;
    freeGlyphMap (map->next);
    free (map->glyphs);
    free (map);
}


static inline ATSGlyphRef glyphForCharacter (GlyphMap *map, UniChar c, NSFont **font)
{
    if (map == 0)
        return nonGlyphID;
        
    if (c >= map->startRange && c <= map->endRange){
        *font = map->glyphs[c-map->startRange].font;
        return map->glyphs[c-map->startRange].glyph;
    }
        
    return glyphForCharacter (map->next, c, font);
}
 
 
static inline ATSGlyphRef glyphForUnicodeCharacter (UnicodeGlyphMap *map, UnicodeChar c, NSFont **font)
{
    if (map == 0)
        return nonGlyphID;
        
    if (c >= map->startRange && c <= map->endRange){
        *font = map->glyphs[c-map->startRange].font;
        return map->glyphs[c-map->startRange].glyph;
    }
        
    return glyphForUnicodeCharacter (map->next, c, font);
}
 

#ifdef _TIMING        
static double totalCGGetAdvancesTime = 0;
#endif

static inline SubstituteFontWidthMap *mapForSubstituteFont(WebTextRenderer *renderer, NSFont *font)
{
    int i;
    
    for (i = 0; i < renderer->numSubstituteFontWidthMaps; i++){
        if (font == renderer->substituteFontWidthMaps[i].font)
            return &renderer->substituteFontWidthMaps[i];
    }
    
    if (renderer->numSubstituteFontWidthMaps == renderer->maxSubstituteFontWidthMaps){
        renderer->maxSubstituteFontWidthMaps = renderer->maxSubstituteFontWidthMaps * 2;
        renderer->substituteFontWidthMaps = realloc (renderer->substituteFontWidthMaps, renderer->maxSubstituteFontWidthMaps * sizeof(SubstituteFontWidthMap));
        for (i = renderer->numSubstituteFontWidthMaps; i < renderer->maxSubstituteFontWidthMaps; i++){
            renderer->substituteFontWidthMaps[i].font = 0;
            renderer->substituteFontWidthMaps[i].map = 0;
        }
    }
    
    renderer->substituteFontWidthMaps[renderer->numSubstituteFontWidthMaps].font = font;
    return &renderer->substituteFontWidthMaps[renderer->numSubstituteFontWidthMaps++];
}

static inline WebGlyphWidth widthFromMap (WebTextRenderer *renderer, WidthMap *map, ATSGlyphRef glyph, NSFont *font)
{
    WebGlyphWidth width = UNINITIALIZED_GLYPH_WIDTH;
    BOOL errorResult;
    
    while (1){
        if (map == 0)
            map = [renderer extendGlyphToWidthMapToInclude: glyph font:font];

        if (glyph >= map->startRange && glyph <= map->endRange){
            width = map->widths[glyph-map->startRange].width;
            if (width == UNINITIALIZED_GLYPH_WIDTH){

#ifdef _TIMING
                double startTime = CFAbsoluteTimeGetCurrent();
#endif
                if (font)
                    errorResult = CGFontGetGlyphScaledAdvances ([font _backingCGSFont], &glyph, 1, &map->widths[glyph-map->startRange].width, [renderer->font pointSize]);
                else
                    errorResult = CGFontGetGlyphScaledAdvances ([renderer->font _backingCGSFont], &glyph, 1, &map->widths[glyph-map->startRange].width, [renderer->font pointSize]);
                if (errorResult == 0)
                    FATAL_ALWAYS ("Unable to cache glyph widths for %@ %f",  [renderer->font displayName], [renderer->font pointSize]);

#ifdef _TIMING
                double thisTime = CFAbsoluteTimeGetCurrent() - startTime;
                totalCGGetAdvancesTime += thisTime;
#endif
                width = map->widths[glyph-map->startRange].width;
            }
        }

        if (width == UNINITIALIZED_GLYPH_WIDTH){
            map = map->next;
            continue;
        }
        
        // Hack to ensure that characters that match the width of the space character
        // have the same integer width as the space character.  This is necessary so
        // glyphs in fixed pitch fonts all have the same integer width.  We can't depend
        // on the fixed pitch property of NSFont because that isn't set for all
        // monospaced fonts, in particular Courier!  This has the downside of inappropriately
        // adjusting the widths of characters in non-monospaced fonts that are coincidentally
        // the same width as a space in that font.  In practice this is not an issue as the
        // adjustment is always as the sub-pixel level.
        if (width == renderer->spaceWidth)
            return renderer->ceiledSpaceWidth;

        return width;
    }
    // never get here.
    return 0;
}    

static inline WebGlyphWidth widthForGlyph (WebTextRenderer *renderer, ATSGlyphRef glyph, NSFont *font)
{
    WidthMap *map;

    if (font && font != renderer->font)
        map = mapForSubstituteFont(renderer, font)->map;
    else
        map = renderer->glyphToWidthMap;

    return widthFromMap (renderer, map, glyph, font);
}

static inline  WebGlyphWidth widthForCharacter (WebTextRenderer *renderer, UniChar c, NSFont **font)
{
    ATSGlyphRef glyphID = glyphForCharacter(renderer->characterToGlyphMap, c, font);
    return widthForGlyph (renderer, glyphID, *font);
}


static BOOL FillStyleWithAttributes(ATSUStyle style, NSFont *theFont)
{
    if (theFont) {
        ATSUFontID fontId = (ATSUFontID)[theFont _atsFontID];
        ATSUAttributeTag tag = kATSUFontTag;
        ByteCount size = sizeof(ATSUFontID);
        ATSUFontID *valueArray[1] = {&fontId};

        if (fontId) {
            if (ATSUSetAttributes(style, 1, &tag, &size, (void *)valueArray) != noErr)
                return NO;
        }
        else {
            return NO;
        }
        return YES;
    }
    return NO;
}


static unsigned int findLengthOfCharacterCluster(const UniChar *characters, unsigned int length)
{
    unsigned int k;

    if (length <= 1)
        return length;
    
    if (IsHighSurrogatePair(characters[0]))
        return 2;
        
    if (IsNonBaseChar(characters[0]))
        return 1;
    
    // Find all the non base characters after the current character.
    for (k = 1; k < length; k++)
        if (!IsNonBaseChar(characters[k]))
            break;
    return k;
}


@implementation WebTextRenderer

static BOOL bufferTextDrawing = NO;

+ (BOOL)shouldBufferTextDrawing
{
    return bufferTextDrawing;
}

+ (void)initialize
{
    nonBaseChars = CFCharacterSetGetPredefined(kCFCharacterSetNonBase);
    bufferTextDrawing = [[[NSUserDefaults standardUserDefaults] stringForKey:@"BufferTextDrawing"] isEqual: @"YES"];
}

static inline BOOL _fontContainsString (NSFont *font, NSString *string)
{
    if ([string rangeOfCharacterFromSet:[[font coveredCharacterSet] invertedSet]].location == NSNotFound) {
        return YES;
    }
    return NO;
}

- (NSFont *)substituteFontForString: (NSString *)string families: (NSString **)families
{
    NSFont *substituteFont = nil;

    // First search the CSS family fallback list.  Start at 1 (2nd font)
    // because we've already failed on the first lookup.
    NSString *family = nil;
    int i = 1;
    while (families && families[i] != 0) {
        family = families[i++];
        substituteFont = [[WebTextRendererFactory sharedFactory] cachedFontFromFamily: family traits:[[NSFontManager sharedFontManager] traitsOfFont:font] size:[font pointSize]];
        if (substituteFont && _fontContainsString(substituteFont, string)){
            return substituteFont;
        }
    }
    
    // Now do string based lookup.
    substituteFont = [NSFont findFontLike:font forString:string withRange:NSMakeRange (0,[string length]) inLanguage:[NSLanguage defaultLanguage]];

    // Now do character based lookup.
    if (substituteFont == nil && [string length] == 1)
        substituteFont = [NSFont findFontLike:font forCharacter: [string characterAtIndex: 0] inLanguage:[NSLanguage defaultLanguage]];
    
    if ([substituteFont isEqual: font])
        substituteFont = nil;
    
    return substituteFont;
}


- (NSFont *)substituteFontForCharacters: (const unichar *)characters length: (int)numCharacters families: (NSString **)families
{
    NSFont *substituteFont;
    NSString *string = [[NSString alloc] initWithCharactersNoCopy:(unichar *)characters length: numCharacters freeWhenDone: NO];

    substituteFont = [self substituteFontForString: string families: families];

    [string release];
    
    return substituteFont;
}


/* Convert non-breaking spaces into spaces, and skip control characters. */
- (void)convertCharacters: (const UniChar *)characters length: (unsigned)numCharacters toGlyphs: (ATSGlyphVector *)glyphs skipControlCharacters:(BOOL)skipControlCharacters
{
    unsigned i, numCharactersInBuffer;
    UniChar localBuffer[LOCAL_BUFFER_SIZE];
    UniChar *buffer = localBuffer;
    OSStatus status;
    
    for (i = 0; i < numCharacters; i++) {
        UniChar c = characters[i];
        if ((skipControlCharacters && IS_CONTROL_CHARACTER(c)) || c == NON_BREAKING_SPACE) {
            break;
        }
    }
    
    if (i < numCharacters) {
        if (numCharacters > LOCAL_BUFFER_SIZE) {
            buffer = (UniChar *)malloc(sizeof(UniChar) * numCharacters);
        }
        
        numCharactersInBuffer = 0;
        for (i = 0; i < numCharacters; i++) {
            UniChar c = characters[i];
            if (c == NON_BREAKING_SPACE) {
                buffer[numCharactersInBuffer++] = SPACE;
            } else if (!(skipControlCharacters && IS_CONTROL_CHARACTER(c))) {
                buffer[numCharactersInBuffer++] = characters[i];
            }
        }
        
        characters = buffer;
        numCharacters = numCharactersInBuffer;
    }
    
    status = ATSUConvertCharToGlyphs(styleGroup, characters, 0, numCharacters, 0, glyphs);
    if (status != noErr){
        FATAL_ALWAYS ("unable to get glyphsfor %@ %f error = (%d)", self, [font displayName], [font pointSize], status);
    }

#ifdef DEBUG_GLYPHS
    int foundGlyphs = 0;
    ATSLayoutRecord *glyphRecord;
    for (i = 0; i < numCharacters; i++) {
        glyphRecord = (ATSLayoutRecord *)glyphs->firstRecord;
        for (i = 0; i < numCharacters; i++) {
            if (glyphRecord->glyphID != 0)
                foundGlyphs++;
            glyphRecord = (ATSLayoutRecord *)((char *)glyphRecord + glyphs->recordSize);
        }
    }
    printf ("For %s found %d glyphs in range 0x%04x to 0x%04x\n", [[font displayName] cString], foundGlyphs, characters[0], characters[numCharacters-1]);
#endif
    if (buffer != localBuffer) {
        free(buffer);
    }
}


- (void)convertUnicodeCharacters: (const UnicodeChar *)characters length: (unsigned)numCharacters toGlyphs: (ATSGlyphVector *)glyphs
{
    unsigned numCharactersInBuffer;
    UniChar localBuffer[LOCAL_BUFFER_SIZE];
    UniChar *buffer = localBuffer;
    OSStatus status;
    unsigned i, bufPos = 0;
    
    if (numCharacters*2 > LOCAL_BUFFER_SIZE) {
        buffer = (UniChar *)malloc(sizeof(UniChar) * numCharacters * 2);
    }
    
    numCharactersInBuffer = 0;
    for (i = 0; i < numCharacters; i++) {
        UnicodeChar c = characters[i];
        UniChar h = HighSurrogatePair(c);
        UniChar l = LowSurrogatePair(c);
        buffer[bufPos++] = h;
        buffer[bufPos++] = l;
    }
        
    status = ATSUConvertCharToGlyphs(styleGroup, buffer, 0, numCharacters*2, 0, glyphs);
    if (status != noErr){
        FATAL_ALWAYS ("unable to get glyphsfor %@ %f error = (%d)", self, [font displayName], [font pointSize], status);
    }
    
    if (buffer != localBuffer) {
        free(buffer);
    }
}

// Nasty hack to determine if we should round or ceil space widths.
// If the font is monospace, or fake monospace we ceil to ensure that 
// every character and the space are the same width.  Otherwise we round.
- (BOOL)_computeWidthForSpace
{
    UniChar c = ' ';
    float _spaceWidth;

    spaceGlyph = [self extendCharacterToGlyphMapToInclude: c];
    if (spaceGlyph == 0){
        return NO;
    }
    _spaceWidth = widthForGlyph(self, spaceGlyph, 0);
    ceiledSpaceWidth = (float)CEIL_TO_INT(_spaceWidth);
    roundedSpaceWidth = (float)ROUND_TO_INT(_spaceWidth);
    if ([font isFixedPitch] || [font _isFakeFixedPitch]){
        adjustedSpaceWidth = ceiledSpaceWidth;
    }
    else {
        adjustedSpaceWidth = roundedSpaceWidth;
    }
    spaceWidth = _spaceWidth;
    
    return YES;
}

- (BOOL)_setupFont: (NSFont *)f
{
    OSStatus errCode;
    ATSUStyle style;
    
    if ((errCode = ATSUCreateStyle(&style)) != noErr)
        return NO;

    if (!FillStyleWithAttributes(style, font))
        return NO;

    if ((errCode = ATSUGetStyleGroup(style, &styleGroup)) != noErr){
        ATSUDisposeStyle(style);
        return NO;
    }
    
    ATSUDisposeStyle(style);

    if (![self _computeWidthForSpace]){
        if (styleGroup){
            ATSUDisposeStyleGroup(styleGroup);
            styleGroup = 0;
        }
        return NO;
    }
   
    return YES;
}

#define ATSFontRefFromNSFont(font) (FMGetATSFontRefFromFont((FMFont)[font _atsFontID]))
static NSString *pathFromFont (NSFont *font)
{
    UInt8 _filePathBuffer[PATH_MAX];
    NSString *filePath = nil;
    FSSpec oFile;
    OSStatus status = ATSFontGetFileSpecification(
            ATSFontRefFromNSFont(font),
            &oFile);
    if (status == noErr){
        OSErr err;
        FSRef fileRef;
        err = FSpMakeFSRef(&oFile,&fileRef);
        if (err == noErr){
            status = FSRefMakePath(&fileRef,_filePathBuffer, PATH_MAX);
            if (status == noErr){
                filePath = [NSString stringWithUTF8String:&_filePathBuffer[0]];
            }
        }
    }
    return filePath;
}

static NSString *WebFallbackFontFamily;

- initWithFont:(NSFont *)f usingPrinterFont:(BOOL)p
{
    if ([f glyphPacking] != NSNativeShortGlyphPacking &&
        [f glyphPacking] != NSTwoByteGlyphPacking)
        FATAL_ALWAYS ("%@: Don't know how to pack glyphs for font %@ %f", self, [f displayName], [f pointSize]);
        
    [super init];
    
    maxSubstituteFontWidthMaps = NUM_SUBSTITUTE_FONT_MAPS;
    substituteFontWidthMaps = calloc (1, maxSubstituteFontWidthMaps * sizeof(SubstituteFontWidthMap));
    font = [(p ? [f printerFont] : [f screenFont]) retain];
    usingPrinterFont = p;
    
    if (![self _setupFont:font]){
        // Ack!  Something very bad happened, like a corrupt font.  Try
        // looking for an alternate 'base' font for this renderer.

        // Special case hack to use "Times New Roman" in place of "Times".  "Times RO" is a common font
        // whose family name is "Times".  It overrides the normal "Times" family font.  It also
        // appears to have a corrupt regular variant.
        NSString *fallbackFontFamily;

        if ([[font familyName] isEqual:@"Times"])
            fallbackFontFamily = @"Times New Roman";
        else {
            if (!WebFallbackFontFamily)
                // Could use any size, we just care about the family of the system font.
                WebFallbackFontFamily = [[[NSFont systemFontOfSize:16.0] familyName] retain];
                
            fallbackFontFamily = WebFallbackFontFamily;
        }
        
        // Try setting up the alternate font.
        NSFont *alternateFont = [[NSFontManager sharedFontManager] convertFont:font toFamily:fallbackFontFamily];
        NSFont *initialFont = font;
        [initialFont autorelease];
        font = [alternateFont retain];
        NSString *filePath = pathFromFont(initialFont);
        filePath = filePath ? filePath : @"not known";
        if (![self _setupFont:alternateFont]){
            // Give up!
            FATAL_ALWAYS ("%@ unable to initialize with font %@ at %@", self, initialFont, filePath);
        }

        // Report the problem.
        ERROR ("Corrupt font detected, using %@ in place of %@ (%d glyphs) located at \"%@\".", 
                    [alternateFont familyName], 
                    [initialFont familyName],
                    ATSFontGetGlyphCount(ATSFontRefFromNSFont(initialFont)),
                    filePath);
    }

    // We emulate the appkit metrics by applying rounding as is done
    // in the appkit.
    CGFontRef cgFont = [f _backingCGSFont];
    const CGFontHMetrics *metrics = CGFontGetHMetrics(cgFont);
    unsigned int unitsPerEm = CGFontGetUnitsPerEm(cgFont);
    float pointSize = [f pointSize];
    float asc = (ScaleEmToUnits(metrics->ascent, unitsPerEm)*pointSize);
    float dsc = (-ScaleEmToUnits(metrics->descent, unitsPerEm)*pointSize);
    float lineGap = ScaleEmToUnits(metrics->lineGap, unitsPerEm)*pointSize;
    float adjustment;

    // We need to adjust Times, Helvetica, and Courier to closely match the
    // vertical metrics of their Microsoft counterparts that are the de facto
    // web standard.  The AppKit adjustment of 20% is too big and is
    // incorrectly added to line spacing, so we use a 15% adjustment instead
    // and add it to the ascent.
    if ([[font familyName] isEqualToString:@"Times"] ||
        [[font familyName] isEqualToString:@"Helvetica"] ||
        [[font familyName] isEqualToString:@"Courier"]) {
        adjustment = floor(((asc + dsc) * 0.15) + 0.5);
    } else {
        adjustment = 0.0;
    }

    ascent = ROUND_TO_INT(asc + adjustment);
    descent = ROUND_TO_INT(dsc);

    lineSpacing =  ascent + descent + (int)(lineGap > 0.0 ? floor(lineGap + 0.5) : 0.0);

#ifdef COMPARE_APPKIT_CG_METRICS
    printf ("\nCG/Appkit metrics for font %s, %f, lineGap %f, adjustment %f, _canDrawOutsideLineHeight %d, _isSystemFont %d\n", [[f displayName] cString], [f pointSize], lineGap, adjustment, (int)[f _canDrawOutsideLineHeight], (int)[f _isSystemFont]);
    if ((int)ROUND_TO_INT([f ascender]) != ascent ||
        (int)ROUND_TO_INT(-[f descender]) != descent ||
        (int)ROUND_TO_INT([font defaultLineHeightForFont]) != lineSpacing){
        printf ("\nCG/Appkit mismatched metrics for font %s, %f (%s)\n", [[f displayName] cString], [f pointSize],
                ([f screenFont] ? [[[f screenFont] displayName] cString] : "none"));
        printf ("ascent(%s), descent(%s), lineSpacing(%s)\n",
                ((int)ROUND_TO_INT([f ascender]) != ascent) ? "different" : "same",
                ((int)ROUND_TO_INT(-[f descender]) != descent) ? "different" : "same",
                ((int)ROUND_TO_INT([font defaultLineHeightForFont]) != lineSpacing) ? "different" : "same");
        printf ("CG:  ascent %f, ", asc);
        printf ("descent %f, ", dsc);
        printf ("lineGap %f, ", lineGap);
        printf ("lineSpacing %d\n", lineSpacing);
        
        printf ("NSFont:  ascent %f, ", [f ascender]);
        printf ("descent %f, ", [f descender]);
        printf ("lineSpacing %f\n", [font defaultLineHeightForFont]);
    }
#endif
     
    return self;
}


- (void)dealloc
{
    [font release];

    if (styleGroup)
        ATSUDisposeStyleGroup(styleGroup);

    freeWidthMap (glyphToWidthMap);
    freeGlyphMap (characterToGlyphMap);
    
    [super dealloc];
}


- (int)widthForCharacters:(const UniChar *)characters length:(unsigned)stringLength
{
    return ROUND_TO_INT([self floatWidthForCharacters:characters stringLength:stringLength fromCharacterPosition:0 numberOfCharacters:stringLength withPadding: 0 applyRounding:YES attemptFontSubstitution: YES widths: 0 letterSpacing: 0 wordSpacing: 0 smallCaps: false fontFamilies: 0]);
}

- (int)widthForString:(NSString *)string
{
    UniChar localCharacterBuffer[LOCAL_BUFFER_SIZE];
    UniChar *characterBuffer = localCharacterBuffer;
    const UniChar *usedCharacterBuffer = CFStringGetCharactersPtr((CFStringRef)string);
    unsigned int length;
    int width;

    // Get the characters from the string into a buffer.
    length = [string length];
    if (!usedCharacterBuffer) {
        if (length > LOCAL_BUFFER_SIZE)
            characterBuffer = (UniChar *)malloc(length * sizeof(UniChar));
        [string getCharacters:characterBuffer];
        usedCharacterBuffer = characterBuffer;
    }

    width = [self widthForCharacters:usedCharacterBuffer length:length];
    
    if (characterBuffer != localCharacterBuffer)
        free(characterBuffer);

    return width;
}


- (int)ascent
{
    return ascent;
}


- (int)descent
{
    return descent;
}


- (int)lineSpacing
{
    return lineSpacing;
}


- (float)xHeight
{
    return [font xHeight];
}


// Useful page for testing http://home.att.net/~jameskass
static void _drawGlyphs(NSFont *font, NSColor *color, CGGlyph *glyphs, CGSize *advances, float x, float y, int numGlyphs)
{
    CGContextRef cgContext;

    if ([WebTextRenderer shouldBufferTextDrawing] && [[WebTextRendererFactory sharedFactory] coalesceTextDrawing]){
        // Add buffered glyphs and advances
        // FIXME:  If we ever use this again, need to add RTL.
        WebGlyphBuffer *gBuffer = [[WebTextRendererFactory sharedFactory] glyphBufferForFont: font andColor: color];
        [gBuffer addGlyphs: glyphs advances: advances count: numGlyphs at: x : y];
    }
    else {
        cgContext = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
        // Setup the color and font.
        [color set];
        [font set];

        CGContextSetTextPosition (cgContext, x, y);
        CGContextShowGlyphsWithAdvances (cgContext, glyphs, advances, numGlyphs);
    }
}

- (void)drawCharacters:(const UniChar *)characters stringLength: (unsigned int)length fromCharacterPosition: (int)from toCharacterPosition: (int)to atPoint:(NSPoint)point withPadding: (int)padding withTextColor:(NSColor *)textColor backgroundColor: (NSColor *)backgroundColor rightToLeft: (BOOL)rtl letterSpacing: (int)letterSpacing wordSpacing: (int)wordSpacing smallCaps: (BOOL)smallCaps fontFamilies: (NSString **)families
{
    float *widthBuffer, localWidthBuffer[LOCAL_BUFFER_SIZE];
    CGGlyph *glyphBuffer, localGlyphBuffer[LOCAL_BUFFER_SIZE];
    NSFont **fontBuffer, *localFontBuffer[LOCAL_BUFFER_SIZE];
    CGSize *advances, localAdvanceBuffer[LOCAL_BUFFER_SIZE];
    int numGlyphs, i;
    float startX, nextX, backgroundWidth = 0.0;
    NSFont *currentFont;
    
    if (length == 0)
        return;

    // WARNING:  the character to glyph translation must result in less than
    // length glyphs.  As of now this is always true.
    if (length > LOCAL_BUFFER_SIZE) {
        advances = (CGSize *)calloc(length, sizeof(CGSize));
        widthBuffer = (float *)calloc(length, sizeof(float));
        glyphBuffer = (CGGlyph *)calloc(length, sizeof(ATSGlyphRef));
        fontBuffer = (NSFont **)calloc(length, sizeof(NSFont *));
    } else {
        advances = localAdvanceBuffer;
        widthBuffer = localWidthBuffer;
        glyphBuffer = localGlyphBuffer;
        fontBuffer = localFontBuffer;
    }

    [self _floatWidthForCharacters:characters 
        stringLength:length 
        fromCharacterPosition: 0 
        numberOfCharacters: length
        withPadding: padding
        applyRounding: YES
        attemptFontSubstitution: YES 
        widths: widthBuffer 
        fonts: fontBuffer
        glyphs: glyphBuffer
        numGlyphs: &numGlyphs
        letterSpacing: letterSpacing
        wordSpacing: wordSpacing
        smallCaps: smallCaps
        fontFamilies: families];

    if (from == -1)
        from = 0;
    if (to == -1)
        to = numGlyphs;

    for (i = 0; (int)i < MIN(to,(int)numGlyphs); i++){
        advances[i].width = widthBuffer[i];
        advances[i].height = 0;
    }

    startX = point.x;
    for (i = 0; (int)i < MIN(from,(int)numGlyphs); i++)
        startX += advances[i].width;

    for (i = from; (int)i < MIN(to,(int)numGlyphs); i++)
        backgroundWidth += advances[i].width;

    if (backgroundColor != nil){
        [backgroundColor set];
        [NSBezierPath fillRect:NSMakeRect(startX, point.y - [self ascent], backgroundWidth, [self lineSpacing])];
    }
    
    // Finally, draw the glyphs.
    if (from < (int)numGlyphs){
        int lastFrom = from;
        int pos = from;

        if (rtl && numGlyphs > 1){
            int i;
            int end = numGlyphs;
            CGGlyph gswap1, gswap2;
            CGSize aswap1, aswap2;
            NSFont *fswap1, *fswap2;
            
            for (i = pos, end = numGlyphs; i < (numGlyphs - pos)/2; i++){
                gswap1 = glyphBuffer[i];
                gswap2 = glyphBuffer[--end];
                glyphBuffer[i] = gswap2;
                glyphBuffer[end] = gswap1;
            }
            for (i = pos, end = numGlyphs; i < (numGlyphs - pos)/2; i++){
                aswap1 = advances[i];
                aswap2 = advances[--end];
                advances[i] = aswap2;
                advances[end] = aswap1;
            }
            for (i = pos, end = numGlyphs; i < (numGlyphs - pos)/2; i++){
                fswap1 = fontBuffer[i];
                fswap2 = fontBuffer[--end];
                fontBuffer[i] = fswap2;
                fontBuffer[end] = fswap1;
            }
        }

        currentFont = fontBuffer[pos];
        nextX = startX;
        int nextGlyph = pos;
        // FIXME:  Don't run over the end of the glyph buffer when the
        // number of glyphs is less than the number of characters.  This
        // happens when a ligature is included in the glyph run.  The code
        // below will just stop drawing glyphs at the end of the glyph array
        // as a temporary hack.  The appropriate fix need to map character
        // ranges to glyph ranges.
        while (nextGlyph < to && nextGlyph < numGlyphs){
            if ((fontBuffer[nextGlyph] != 0 && fontBuffer[nextGlyph] != currentFont)){
                _drawGlyphs(currentFont, textColor, &glyphBuffer[lastFrom], &advances[lastFrom], startX, point.y, nextGlyph - lastFrom);
                lastFrom = nextGlyph;
                currentFont = fontBuffer[nextGlyph];
                startX = nextX;
            }
            nextX += advances[nextGlyph].width;
            nextGlyph++;
        }
        _drawGlyphs(currentFont, textColor, &glyphBuffer[lastFrom], &advances[lastFrom], startX, point.y, nextGlyph - lastFrom);
    }

    if (advances != localAdvanceBuffer) {
        free(advances);
        free(widthBuffer);
        free(glyphBuffer);
        free(fontBuffer);
    }
}


- (void)drawLineForCharacters:(NSPoint)point yOffset:(float)yOffset withWidth: (int)width withColor:(NSColor *)color
{
    NSGraphicsContext *graphicsContext = [NSGraphicsContext currentContext];
    CGContextRef cgContext;
    float lineWidth;

    // This will draw the text from the top of the bounding box down.
    // Qt expects to draw from the baseline.
    // Remember that descender is negative.
    point.y -= [self lineSpacing] - [self descent];
    
    BOOL flag = [graphicsContext shouldAntialias];

    [graphicsContext setShouldAntialias: NO];

    [color set];

    cgContext = (CGContextRef)[graphicsContext graphicsPort];
    lineWidth = 0.0;
    if ([graphicsContext isDrawingToScreen] && lineWidth == 0.0) {
        CGSize size = CGSizeApplyAffineTransform(CGSizeMake(1.0, 1.0), CGAffineTransformInvert(CGContextGetCTM(cgContext)));
        lineWidth = size.width;
    }
    CGContextSetLineWidth(cgContext, lineWidth);
    CGContextMoveToPoint(cgContext, point.x, point.y + [self lineSpacing] + 1.5 - [self descent] + yOffset);
    CGContextAddLineToPoint(cgContext, point.x + width, point.y + [self lineSpacing] + 1.5 - [self descent] + yOffset);
    CGContextStrokePath(cgContext);

    [graphicsContext setShouldAntialias: flag];
}


- (float)floatWidthForCharacters:(const UniChar *)characters stringLength:(unsigned)stringLength characterPosition: (int)pos
{
    // Return the width of the first complete character at the specified position.  Even though
    // the first 'character' may contain more than one unicode characters this method will
    // work correctly.
    return [self floatWidthForCharacters:characters stringLength:stringLength fromCharacterPosition:pos numberOfCharacters:1 withPadding: 0 applyRounding: YES attemptFontSubstitution: YES widths: nil letterSpacing: 0 wordSpacing: 0 smallCaps: false fontFamilies: 0];
}


- (float)floatWidthForCharacters:(const UniChar *)characters stringLength:(unsigned)stringLength fromCharacterPosition: (int)pos numberOfCharacters: (int)len
{
    return [self floatWidthForCharacters:characters stringLength:stringLength fromCharacterPosition:pos numberOfCharacters:len withPadding: 0 applyRounding: YES attemptFontSubstitution: YES widths: nil letterSpacing: 0 wordSpacing: 0 smallCaps: false fontFamilies: 0];
}


- (float)floatWidthForCharacters:(const UniChar *)characters stringLength:(unsigned)stringLength fromCharacterPosition: (int)pos numberOfCharacters: (int)len withPadding: (int)padding applyRounding: (BOOL)applyRounding attemptFontSubstitution: (BOOL)attemptSubstitution widths: (float *)widthBuffer letterSpacing: (int)letterSpacing wordSpacing: (int)wordSpacing smallCaps: (BOOL)smallCaps fontFamilies: (NSString **)families
{
    return [self _floatWidthForCharacters:characters stringLength:stringLength fromCharacterPosition:pos numberOfCharacters:len withPadding: padding applyRounding: YES attemptFontSubstitution: YES widths: widthBuffer fonts: nil  glyphs: nil numGlyphs: nil letterSpacing: letterSpacing wordSpacing: wordSpacing smallCaps: smallCaps fontFamilies: families];
}

#ifdef DEBUG_COMBINING
static const char *directionNames[] = {
        "DirectionL", 	// Left Letter 
        "DirectionR",	// Right Letter
        "DirectionEN",	// European Number
        "DirectionES",	// European Separator
        "DirectionET",	// European Terminator (post/prefix e.g. $ and %)
        "DirectionAN",	// Arabic Number
        "DirectionCS",	// Common Separator 
        "DirectionB", 	// Paragraph Separator (aka as PS)
        "DirectionS", 	// Segment Separator (TAB)
        "DirectionWS", 	// White space
        "DirectionON",	// Other Neutral

	// types for explicit controls
        "DirectionLRE", 
        "DirectionLRO", 

        "DirectionAL", 	// Arabic Letter (Right-to-left)

        "DirectionRLE", 
        "DirectionRLO", 
        "DirectionPDF", 

        "DirectionNSM", 	// Non-spacing Mark
        "DirectionBN"	// Boundary neutral (type of RLE etc after explicit levels)
};

static const char *joiningNames[] = {
        "JoiningOther",
        "JoiningDual",
        "JoiningRight",
        "JoiningCausing"
};
#endif

- (float)_floatWidthForCharacters:(const UniChar *)characters stringLength:(unsigned)stringLength fromCharacterPosition: (int)pos numberOfCharacters: (int)len withPadding: (int)padding applyRounding: (BOOL)applyRounding attemptFontSubstitution: (BOOL)attemptSubstitution widths: (float *)widthBuffer fonts: (NSFont **)fontBuffer glyphs: (CGGlyph *)glyphBuffer numGlyphs: (int *)_numGlyphs letterSpacing: (int)letterSpacing wordSpacing: (int)wordSpacing smallCaps: (BOOL)smallCaps fontFamilies: (NSString **)families
{
    float totalWidth = 0;
    unsigned int i, clusterLength;
    NSFont *substituteFont = nil;
    ATSGlyphRef glyphID;
    float lastWidth = 0;
    uint numSpaces = 0;
    int padPerSpace = 0;
    int numGlyphs = 0;
    UniChar high = 0, low = 0;

    if (len <= 0){
        if (_numGlyphs)
            *_numGlyphs = 0;
        return 0;
    }

#define SHAPE_STRINGS
#ifdef SHAPE_STRINGS
     UniChar *munged = 0;
     UniChar _localMunged[LOCAL_BUFFER_SIZE];
    {
        UniChar *shaped;
        int lengthOut;
        shaped = shapedString ((UniChar *)characters, stringLength,
                               pos,
                               len,
                               0, &lengthOut);
        if (shaped){
             if (stringLength < LOCAL_BUFFER_SIZE)
                 munged = &_localMunged[0];
             else
                 munged = (UniChar *)malloc(stringLength * sizeof(UniChar));
            //printf ("%d input, %d output\n", len, lengthOut);
            //for (i = 0; i < (int)len; i++){
            //    printf ("0x%04x shaped to 0x%04x\n", characters[i], shaped[i]);
            //}
            // FIXME:  Hack-o-rific, copy shaped buffer into munged
            // character buffer.
            for (i = 0; i < (unsigned int)pos; i++){
                munged[i] = characters[i];
            }
            for (i = pos; i < (unsigned int)pos+lengthOut; i++){
                munged[i] = shaped[i-pos];
            }
            characters = munged;
            len = lengthOut;
        }
    }
#endif
    
    // If the padding is non-zero, count the number of spaces in the string
    // and divide that by the padding for per space addition.
    if (padding > 0){
        for (i = pos; i < (uint)pos+len; i++){
            if (characters[i] == NON_BREAKING_SPACE || characters[i] == SPACE)
                numSpaces++;
        }
        padPerSpace = CEIL_TO_INT ((((float)padding) / ((float)numSpaces)));
    }

    for (i = 0; i < stringLength; i++) {

        UniChar c = characters[i];
        BOOL foundMetrics = NO;
        
        // Skip control characters.
        if (IS_CONTROL_CHARACTER(c)) {
            continue;
        }
        
        if (c == NON_BREAKING_SPACE) {
            c = SPACE;
        }
        
        // Drop out early if we've measured to the end of the requested
        // fragment.
        if ((int)i - pos >= len) {
            // Check if next character is a space. If so, we have to apply rounding.
            if (c == SPACE && applyRounding) {
                float delta = CEIL_TO_INT(totalWidth) - totalWidth;
                totalWidth += delta;
                if (widthBuffer && numGlyphs > 0)
                    widthBuffer[numGlyphs - 1] += delta;
            }
            break;
        }

        // Deal with surrogate pairs
        if (c >= HighSurrogateRangeStart && c <= HighSurrogateRangeEnd){
            high = c;

            // Make sure we have another character and it's a low surrogate.
            if (i+1 >= stringLength || !IsLowSurrogatePair((low = characters[++i]))){
                // Error!  Use 0 glyph.
                glyphID = 0;
            }
            else {
                UnicodeChar uc = UnicodeValueForSurrogatePair(high, low);
                glyphID = glyphForUnicodeCharacter(unicodeCharacterToGlyphMap, uc, &substituteFont);
                if (glyphID == nonGlyphID) {
                    glyphID = [self extendUnicodeCharacterToGlyphMapToInclude: uc];
                }
            }
        }
        // Otherwise we have a valid 16bit unicode character.
        else {
            glyphID = glyphForCharacter(characterToGlyphMap, c, &substituteFont);
            if (glyphID == nonGlyphID) {
                glyphID = [self extendCharacterToGlyphMapToInclude: c];
            }
        }

        // FIXME:  look at next character to determine if it is non-base, then
        // determine the characters cluster from this base character.
#ifdef DEBUG_DIACRITICAL
        if (IsNonBaseChar(c)){
            printf ("NonBaseCharacter 0x%04x, joining attribute %d(%s), combining class %d, direction %d, glyph %d, width %f\n", c, WebCoreUnicodeJoiningFunction(c), joiningNames(WebCoreUnicodeJoiningFunction(c)), WebCoreUnicodeCombiningClassFunction(c), WebCoreUnicodeDirectionFunction(c), glyphID, widthForGlyph(self, glyphID));
        }
#endif
        
        // Try to find a substitute font if this font didn't have a glyph for a character in the
        // string.  If one isn't found we end up drawing and measuring the 0 glyph, usually a box.
        if (glyphID == 0 && attemptSubstitution) {
            const UniChar *_characters;
            UniChar surrogates[2];
            
            if (high && low){
                clusterLength = 2;
                surrogates[0] = high;
                surrogates[1] = low;
                _characters = &surrogates[0];
            }
            else {
                clusterLength = findLengthOfCharacterCluster (&characters[i], stringLength - i);
                _characters = &characters[i];
            }
            substituteFont = [self substituteFontForCharacters: _characters length: clusterLength families: families];
            if (substituteFont) {
                int cNumGlyphs;
                ATSGlyphRef localGlyphBuffer[clusterLength*4];
                
                lastWidth = [[[WebTextRendererFactory sharedFactory] rendererWithFont:substituteFont usingPrinterFont:usingPrinterFont]
                                _floatWidthForCharacters: _characters 
                                stringLength: clusterLength 
                                fromCharacterPosition: 0 numberOfCharacters: clusterLength 
                                withPadding: 0 applyRounding: NO attemptFontSubstitution: NO 
                                widths: ((widthBuffer != 0 ) ? (&widthBuffer[numGlyphs]) : nil)
                                fonts: nil
                                glyphs: &localGlyphBuffer[0]
                                numGlyphs: &cNumGlyphs
                                letterSpacing: letterSpacing
                                wordSpacing: wordSpacing
                                smallCaps: smallCaps
                                fontFamilies: families];
                
                int j;
                if (glyphBuffer){
                    for (j = 0; j < cNumGlyphs; j++)
                        glyphBuffer[numGlyphs+j] = localGlyphBuffer[j];
                }
                
                if (fontBuffer){
                    for (j = 0; j < cNumGlyphs; j++)
                        fontBuffer[numGlyphs+j] = substituteFont;
                }
                
                if (clusterLength == 1 && cNumGlyphs == 1 && localGlyphBuffer[0] != 0){
                    [self updateGlyphEntryForCharacter: _characters[0] glyphID: localGlyphBuffer[0] font: substituteFont];
                }

                numGlyphs += cNumGlyphs;
                
                foundMetrics = YES;
            }
#ifdef DEBUG_MISSING_GLYPH
            else {
                BOOL hasFont = [[NSFont coveredCharacterCache] characterIsMember:c];
                printf ("Unable to find glyph for base character 0x%04x in font %s(%s) hasFont1 = %d\n", c, [[font displayName] cString], [[substituteFont displayName] cString], (int)hasFont);
            }
#endif
        }
        
        // If we have a valid glyph OR if we couldn't find a substitute font
        // measure the glyph.
        if ((glyphID > 0 || ((glyphID == 0) && substituteFont == nil)) && !foundMetrics) {
            if (glyphID == spaceGlyph && applyRounding) {
                if (lastWidth > 0){
                    float delta = CEIL_TO_INT(totalWidth) - totalWidth;
                    totalWidth += delta;
                    if (widthBuffer)
                        widthBuffer[numGlyphs - 1] += delta;
                } 
                lastWidth = adjustedSpaceWidth;
                if (padding > 0){
                    // Only use left over padding if note evenly divisible by 
                    // number of spaces.
                    if (padding < padPerSpace){
                        lastWidth += padding;
                        padding = 0;
                    }
                    else {
                        lastWidth += padPerSpace;
                        padding -= padPerSpace;
                    }
                }
            }
            else
                lastWidth = widthForGlyph(self, glyphID, substituteFont);
            
            if (fontBuffer){
                fontBuffer[numGlyphs] = (substituteFont ? substituteFont: font);
            }
            if (glyphBuffer)
                glyphBuffer[numGlyphs] = glyphID;

            // Account for the letter-spacing.  Only add width to base characters.
            // Combining glyphs should have zero width.
            if (lastWidth > 0)
                lastWidth += letterSpacing;

            // Account for word-spacing.  Make the size of the last character (grapheme cluster)
            // in the word wider.
            if (glyphID == spaceGlyph && numGlyphs > 0 && (characters[i-1] != SPACE || characters[i-1] != NON_BREAKING_SPACE)){
                // Find the base glyph in the grapheme cluster.  Combining glyphs
                // should have zero width.
                if (widthBuffer){
                    int ng = numGlyphs-1;
                    if (ng >= 0){
                        while (ng && widthBuffer[ng] == 0)
                            ng--;
                        widthBuffer[ng] += wordSpacing;
                    }
                }
                totalWidth += wordSpacing;
            }
            
            if (widthBuffer){
                widthBuffer[numGlyphs] = lastWidth;
            }
            numGlyphs++;
        }
#ifdef DEBUG_COMBINING        
        printf ("Character 0x%04x, joining attribute %d(%s), combining class %d, direction %d(%s)\n", c, WebCoreUnicodeJoiningFunction(c), joiningNames[WebCoreUnicodeJoiningFunction(c)], WebCoreUnicodeCombiningClassFunction(c), WebCoreUnicodeDirectionFunction(c), directionNames[WebCoreUnicodeDirectionFunction(c)]);
#endif

        if (i >= (unsigned int)pos)
            totalWidth += lastWidth;       
    }

    // Ceil the last glyph, but only if
    // 1) The string is longer than one character
    // 2) or the entire stringLength is one character
    if ((len > 1 || stringLength == 1) && applyRounding){
        float delta = CEIL_TO_INT(totalWidth) - totalWidth;
        totalWidth += delta;
        if (widthBuffer && numGlyphs > 0)
            widthBuffer[numGlyphs-1] += delta;
    }

    if (_numGlyphs)
        *_numGlyphs = numGlyphs;

#ifdef SHAPE_STRINGS
     if (munged != &_localMunged[0])
         free (munged);
#endif
    
    return totalWidth;
}

- (ATSGlyphRef)extendUnicodeCharacterToGlyphMapToInclude: (UnicodeChar)c
{
    UnicodeGlyphMap *map = (UnicodeGlyphMap *)calloc (1, sizeof(UnicodeGlyphMap));
    ATSLayoutRecord *glyphRecord;
    ATSGlyphVector glyphVector;
    UnicodeChar end, start;
    unsigned int blockSize;
    ATSGlyphRef glyphID;
    
    if (unicodeCharacterToGlyphMap == 0)
        blockSize = INITIAL_BLOCK_SIZE;
    else
        blockSize = INCREMENTAL_BLOCK_SIZE;
    start = (c / blockSize) * blockSize;
    end = start + (blockSize - 1);
        
    LOG(FontCache, "%@ (0x%04x) adding glyphs for 0x%04x to 0x%04x", font, c, start, end);

    map->startRange = start;
    map->endRange = end;
    
    unsigned int i, count = end - start + 1;
    UnicodeChar buffer[INCREMENTAL_BLOCK_SIZE+2];
    
    for (i = 0; i < count; i++){
        buffer[i] = i+start;
    }

    OSStatus status;
    status = ATSInitializeGlyphVector(count*2, 0, &glyphVector);
    if (status != noErr){
        // This should never happen, indicates a bad font!  If it does the
        // font substitution code will find an alternate font.
        return 0;
    }
    
    [self convertUnicodeCharacters: &buffer[0] length: count toGlyphs: &glyphVector];
    unsigned numGlyphs = glyphVector.numGlyphs;
    if (numGlyphs != count){
        // This should never happen, indicates a bad font!  If it does the
        // font substitution code will find an alternate font.
        return 0;
    }
            
    map->glyphs = (GlyphEntry *)malloc (count * sizeof(GlyphEntry));
    glyphRecord = (ATSLayoutRecord *)glyphVector.firstRecord;
    for (i = 0; i < count; i++) {
        map->glyphs[i].glyph = glyphRecord->glyphID;
        map->glyphs[i].font = 0;
        glyphRecord = (ATSLayoutRecord *)((char *)glyphRecord + glyphVector.recordSize);
    }
    ATSClearGlyphVector(&glyphVector);
    
    if (unicodeCharacterToGlyphMap == 0)
        unicodeCharacterToGlyphMap = map;
    else {
        UnicodeGlyphMap *lastMap = unicodeCharacterToGlyphMap;
        while (lastMap->next != 0)
            lastMap = lastMap->next;
        lastMap->next = map;
    }

    glyphID = map->glyphs[c - start].glyph;
    
    return glyphID;
}

- (void)updateGlyphEntryForCharacter: (UniChar)c glyphID: (ATSGlyphRef)glyphID font: (NSFont *)substituteFont
{
    GlyphMap *lastMap = characterToGlyphMap;
    while (lastMap != 0){
        if (c >= lastMap->startRange && c <= lastMap->endRange){
            lastMap->glyphs[c - lastMap->startRange].glyph = glyphID;
            // This font will leak.  No problem though, it has to stick around
            // forever.  Max theoretical retain counts applied here will be
            // num_fonts_on_system * num_glyphs_in_font.
            lastMap->glyphs[c - lastMap->startRange].font = [substituteFont retain];
            break;
        }
        lastMap = lastMap->next;
    }
}

- (ATSGlyphRef)extendCharacterToGlyphMapToInclude:(UniChar) c
{
    GlyphMap *map = (GlyphMap *)calloc (1, sizeof(GlyphMap));
    ATSLayoutRecord *glyphRecord;
    ATSGlyphVector glyphVector;
    UniChar end, start;
    unsigned int blockSize;
    ATSGlyphRef glyphID;
    
    if (characterToGlyphMap == 0)
        blockSize = INITIAL_BLOCK_SIZE;
    else
        blockSize = INCREMENTAL_BLOCK_SIZE;
    start = (c / blockSize) * blockSize;
    end = start + (blockSize - 1);
        
    LOG(FontCache, "%@ (0x%04x) adding glyphs for 0x%04x to 0x%04x", font, c, start, end);

    map->startRange = start;
    map->endRange = end;
    
    unsigned int i, count = end - start + 1;
    short unsigned int buffer[INCREMENTAL_BLOCK_SIZE+2];
    
    for (i = 0; i < count; i++){
        buffer[i] = i+start;
    }

    [font set];
    OSStatus status = ATSInitializeGlyphVector(count, 0, &glyphVector);
    if (status != noErr){
        // This should never happen, perhaps indicates a bad font!  If it does the
        // font substitution code will find an alternate font.
        return 0;
    }

    [self convertCharacters: &buffer[0] length: count toGlyphs: &glyphVector skipControlCharacters: NO];
    unsigned numGlyphs = glyphVector.numGlyphs;
    if (numGlyphs != count){
        // This should never happen, perhaps indicates a bad font!  If it does the
        // font substitution code will find an alternate font.
        return 0;
    }
            
    map->glyphs = (GlyphEntry *)malloc (count * sizeof(GlyphEntry));
    glyphRecord = (ATSLayoutRecord *)glyphVector.firstRecord;
    for (i = 0; i < count; i++) {
        map->glyphs[i].glyph = glyphRecord->glyphID;
        map->glyphs[i].font = 0;
        glyphRecord = (ATSLayoutRecord *)((char *)glyphRecord + glyphVector.recordSize);
    }
    ATSClearGlyphVector(&glyphVector);
    
    if (characterToGlyphMap == 0)
        characterToGlyphMap = map;
    else {
        GlyphMap *lastMap = characterToGlyphMap;
        while (lastMap->next != 0)
            lastMap = lastMap->next;
        lastMap->next = map;
    }

    glyphID = map->glyphs[c - start].glyph;
    
    // Special case for characters 007F-00A0.
    if (glyphID == 0 && c >= 0x7F && c <= 0xA0){
        glyphID = [font _defaultGlyphForChar: c];
        map->glyphs[c - start].glyph = glyphID;
        map->glyphs[c - start].font = 0;
    }

    return glyphID;
}


- (WidthMap *)extendGlyphToWidthMapToInclude:(ATSGlyphRef)glyphID font:(NSFont *)subFont
{
    WidthMap *map = (WidthMap *)calloc (1, sizeof(WidthMap)), **rootMap;
    unsigned int end;
    ATSGlyphRef start;
    unsigned int blockSize;
    unsigned int i, count;
    
    if (subFont && subFont != font)
        rootMap = &mapForSubstituteFont(self,subFont)->map;
    else
        rootMap = &glyphToWidthMap;
        
    if (*rootMap == 0){
        if ([(subFont ? subFont : font) numberOfGlyphs] < INITIAL_BLOCK_SIZE)
            blockSize = [font numberOfGlyphs];
         else
            blockSize = INITIAL_BLOCK_SIZE;
    }
    else
        blockSize = INCREMENTAL_BLOCK_SIZE;
    start = (glyphID / blockSize) * blockSize;
    end = ((unsigned int)start) + blockSize; 
    if (end > 0xffff)
        end = 0xffff;

    LOG(FontCache, "%@ (0x%04x) adding widths for range 0x%04x to 0x%04x", font, glyphID, start, end);

    map->startRange = start;
    map->endRange = end;
    count = end - start + 1;

    map->widths = (WidthEntry *)malloc (count * sizeof(WidthEntry));

    for (i = 0; i < count; i++){
        map->widths[i].width = UNINITIALIZED_GLYPH_WIDTH;
    }

    if (*rootMap == 0)
        *rootMap = map;
    else {
        WidthMap *lastMap = *rootMap;
        while (lastMap->next != 0)
            lastMap = lastMap->next;
        lastMap->next = map;
    }

#ifdef _TIMING
    LOG(FontCache, "%@ total time to advances lookup %f seconds", font, totalCGGetAdvancesTime);
#endif
    return map;
}

@end
