/* ANSI-C code produced by gperf version 2.7.2 */
/* Command-line: gperf -a -L ANSI-C -E -C -c -o -t -k '*' -NfindProp -Hhash_prop -Wwordlist_prop -D -s 2 cssproperties.gperf  */
/* This file is automatically generated from cssproperties.in by makeprop, do not edit */
/* Copyright 1999 W. Bastian */
#include "cssproperties.h"
struct props {
    const char *name;
    int id;
};
/* maximum key range = 1488, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash_prop (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493,    0, 1493, 1493, 1493, 1493,
      1493,    0, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493,   35,   35,    0,
         0,    0,  215,   10,    0,    0,    0,   15,    0,   35,
       460,    0,   20,   55,    0,   95,    0,   60,   45,  310,
       170,  190,    0, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493, 1493,
      1493, 1493, 1493, 1493, 1493, 1493
    };
  register int hval = len;

  switch (hval)
    {
      default:
      case 32:
        hval += asso_values[(unsigned char)str[31]];
      case 31:
        hval += asso_values[(unsigned char)str[30]];
      case 30:
        hval += asso_values[(unsigned char)str[29]];
      case 29:
        hval += asso_values[(unsigned char)str[28]];
      case 28:
        hval += asso_values[(unsigned char)str[27]];
      case 27:
        hval += asso_values[(unsigned char)str[26]];
      case 26:
        hval += asso_values[(unsigned char)str[25]];
      case 25:
        hval += asso_values[(unsigned char)str[24]];
      case 24:
        hval += asso_values[(unsigned char)str[23]];
      case 23:
        hval += asso_values[(unsigned char)str[22]];
      case 22:
        hval += asso_values[(unsigned char)str[21]];
      case 21:
        hval += asso_values[(unsigned char)str[20]];
      case 20:
        hval += asso_values[(unsigned char)str[19]];
      case 19:
        hval += asso_values[(unsigned char)str[18]];
      case 18:
        hval += asso_values[(unsigned char)str[17]];
      case 17:
        hval += asso_values[(unsigned char)str[16]];
      case 16:
        hval += asso_values[(unsigned char)str[15]];
      case 15:
        hval += asso_values[(unsigned char)str[14]];
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      case 11:
        hval += asso_values[(unsigned char)str[10]];
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

#ifdef __GNUC__
__inline
#endif
const struct props *
findProp (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 157,
      MIN_WORD_LENGTH = 3,
      MAX_WORD_LENGTH = 32,
      MIN_HASH_VALUE = 5,
      MAX_HASH_VALUE = 1492
    };

  static const struct props wordlist_prop[] =
    {
      {"color", CSS_PROP_COLOR},
      {"right", CSS_PROP_RIGHT},
      {"height", CSS_PROP_HEIGHT},
      {"top", CSS_PROP_TOP},
      {"clip", CSS_PROP_CLIP},
      {"clear", CSS_PROP_CLEAR},
      {"border", CSS_PROP_BORDER},
      {"border-color", CSS_PROP_BORDER_COLOR},
      {"border-right", CSS_PROP_BORDER_RIGHT},
      {"border-right-color", CSS_PROP_BORDER_RIGHT_COLOR},
      {"border-top", CSS_PROP_BORDER_TOP},
      {"page", CSS_PROP_PAGE},
      {"border-top-color", CSS_PROP_BORDER_TOP_COLOR},
      {"bottom", CSS_PROP_BOTTOM},
      {"size", CSS_PROP_SIZE},
      {"border-bottom", CSS_PROP_BORDER_BOTTOM},
      {"border-bottom-color", CSS_PROP_BORDER_BOTTOM_COLOR},
      {"cursor", CSS_PROP_CURSOR},
      {"scrollbar-3dlight-color", CSS_PROP_SCROLLBAR_3DLIGHT_COLOR},
      {"border-collapse", CSS_PROP_BORDER_COLLAPSE},
      {"scrollbar-highlight-color", CSS_PROP_SCROLLBAR_HIGHLIGHT_COLOR},
      {"quotes", CSS_PROP_QUOTES},
      {"left", CSS_PROP_LEFT},
      {"scrollbar-track-color", CSS_PROP_SCROLLBAR_TRACK_COLOR},
      {"-khtml-marquee", CSS_PROP__KHTML_MARQUEE},
      {"opacity", CSS_PROP_OPACITY},
      {"float", CSS_PROP_FLOAT},
      {"max-height", CSS_PROP_MAX_HEIGHT},
      {"border-left", CSS_PROP_BORDER_LEFT},
      {"-khtml-user-drag", CSS_PROP__KHTML_USER_DRAG},
      {"border-left-color", CSS_PROP_BORDER_LEFT_COLOR},
      {"width", CSS_PROP_WIDTH},
      {"-khtml-user-select", CSS_PROP__KHTML_USER_SELECT},
      {"border-style", CSS_PROP_BORDER_STYLE},
      {"-khtml-box-pack", CSS_PROP__KHTML_BOX_PACK},
      {"display", CSS_PROP_DISPLAY},
      {"border-right-style", CSS_PROP_BORDER_RIGHT_STYLE},
      {"empty-cells", CSS_PROP_EMPTY_CELLS},
      {"border-top-style", CSS_PROP_BORDER_TOP_STYLE},
      {"border-width", CSS_PROP_BORDER_WIDTH},
      {"table-layout", CSS_PROP_TABLE_LAYOUT},
      {"-khtml-marquee-speed", CSS_PROP__KHTML_MARQUEE_SPEED},
      {"border-right-width", CSS_PROP_BORDER_RIGHT_WIDTH},
      {"visibility", CSS_PROP_VISIBILITY},
      {"border-top-width", CSS_PROP_BORDER_TOP_WIDTH},
      {"list-style", CSS_PROP_LIST_STYLE},
      {"border-bottom-style", CSS_PROP_BORDER_BOTTOM_STYLE},
      {"page-break-after", CSS_PROP_PAGE_BREAK_AFTER},
      {"page-break-before", CSS_PROP_PAGE_BREAK_BEFORE},
      {"border-bottom-width", CSS_PROP_BORDER_BOTTOM_WIDTH},
      {"scrollbar-face-color", CSS_PROP_SCROLLBAR_FACE_COLOR},
      {"direction", CSS_PROP_DIRECTION},
      {"white-space", CSS_PROP_WHITE_SPACE},
      {"list-style-image", CSS_PROP_LIST_STYLE_IMAGE},
      {"line-height", CSS_PROP_LINE_HEIGHT},
      {"min-height", CSS_PROP_MIN_HEIGHT},
      {"outline", CSS_PROP_OUTLINE},
      {"scrollbar-arrow-color", CSS_PROP_SCROLLBAR_ARROW_COLOR},
      {"padding", CSS_PROP_PADDING},
      {"outline-color", CSS_PROP_OUTLINE_COLOR},
      {"-khtml-marquee-style", CSS_PROP__KHTML_MARQUEE_STYLE},
      {"margin", CSS_PROP_MARGIN},
      {"padding-right", CSS_PROP_PADDING_RIGHT},
      {"border-left-style", CSS_PROP_BORDER_LEFT_STYLE},
      {"-apple-text-size-adjust", CSS_PROP__APPLE_TEXT_SIZE_ADJUST},
      {"padding-top", CSS_PROP_PADDING_TOP},
      {"max-width", CSS_PROP_MAX_WIDTH},
      {"margin-right", CSS_PROP_MARGIN_RIGHT},
      {"unicode-bidi", CSS_PROP_UNICODE_BIDI},
      {"margin-top", CSS_PROP_MARGIN_TOP},
      {"border-left-width", CSS_PROP_BORDER_LEFT_WIDTH},
      {"overflow", CSS_PROP_OVERFLOW},
      {"position", CSS_PROP_POSITION},
      {"vertical-align", CSS_PROP_VERTICAL_ALIGN},
      {"list-style-type", CSS_PROP_LIST_STYLE_TYPE},
      {"padding-bottom", CSS_PROP_PADDING_BOTTOM},
      {"orphans", CSS_PROP_ORPHANS},
      {"text-shadow", CSS_PROP_TEXT_SHADOW},
      {"caption-side", CSS_PROP_CAPTION_SIDE},
      {"margin-bottom", CSS_PROP_MARGIN_BOTTOM},
      {"background", CSS_PROP_BACKGROUND},
      {"-khtml-flow-mode", CSS_PROP__KHTML_FLOW_MODE},
      {"scrollbar-shadow-color", CSS_PROP_SCROLLBAR_SHADOW_COLOR},
      {"counter-reset", CSS_PROP_COUNTER_RESET},
      {"background-color", CSS_PROP_BACKGROUND_COLOR},
      {"letter-spacing", CSS_PROP_LETTER_SPACING},
      {"z-index", CSS_PROP_Z_INDEX},
      {"-apple-line-clamp", CSS_PROP__APPLE_LINE_CLAMP},
      {"-khtml-box-flex", CSS_PROP__KHTML_BOX_FLEX},
      {"-khtml-user-modify", CSS_PROP__KHTML_USER_MODIFY},
      {"border-spacing", CSS_PROP_BORDER_SPACING},
      {"font", CSS_PROP_FONT},
      {"text-decoration", CSS_PROP_TEXT_DECORATION},
      {"scrollbar-darkshadow-color", CSS_PROP_SCROLLBAR_DARKSHADOW_COLOR},
      {"word-wrap", CSS_PROP_WORD_WRAP},
      {"text-align", CSS_PROP_TEXT_ALIGN},
      {"background-repeat", CSS_PROP_BACKGROUND_REPEAT},
      {"text-overline", CSS_PROP_TEXT_OVERLINE},
      {"text-overline-color", CSS_PROP_TEXT_OVERLINE_COLOR},
      {"background-image", CSS_PROP_BACKGROUND_IMAGE},
      {"text-line-through", CSS_PROP_TEXT_LINE_THROUGH},
      {"-khtml-marquee-direction", CSS_PROP__KHTML_MARQUEE_DIRECTION},
      {"widows", CSS_PROP_WIDOWS},
      {"page-break-inside", CSS_PROP_PAGE_BREAK_INSIDE},
      {"text-line-through-color", CSS_PROP_TEXT_LINE_THROUGH_COLOR},
      {"-khtml-padding-start", CSS_PROP__KHTML_PADDING_START},
      {"text-overline-mode", CSS_PROP_TEXT_OVERLINE_MODE},
      {"-khtml-box-orient", CSS_PROP__KHTML_BOX_ORIENT},
      {"-khtml-box-direction", CSS_PROP__KHTML_BOX_DIRECTION},
      {"-khtml-margin-start", CSS_PROP__KHTML_MARGIN_START},
      {"-khtml-marquee-repetition", CSS_PROP__KHTML_MARQUEE_REPETITION},
      {"-khtml-box-flex-group", CSS_PROP__KHTML_BOX_FLEX_GROUP},
      {"padding-left", CSS_PROP_PADDING_LEFT},
      {"text-overflow", CSS_PROP_TEXT_OVERFLOW},
      {"text-line-through-mode", CSS_PROP_TEXT_LINE_THROUGH_MODE},
      {"-khtml-margin-collapse", CSS_PROP__KHTML_MARGIN_COLLAPSE},
      {"margin-left", CSS_PROP_MARGIN_LEFT},
      {"-apple-dashboard-region", CSS_PROP__APPLE_DASHBOARD_REGION},
      {"-khtml-box-align", CSS_PROP__KHTML_BOX_ALIGN},
      {"font-size", CSS_PROP_FONT_SIZE},
      {"font-stretch", CSS_PROP_FONT_STRETCH},
      {"-khtml-margin-top-collapse", CSS_PROP__KHTML_MARGIN_TOP_COLLAPSE},
      {"min-width", CSS_PROP_MIN_WIDTH},
      {"-khtml-border-vertical-spacing", CSS_PROP__KHTML_BORDER_VERTICAL_SPACING},
      {"outline-style", CSS_PROP_OUTLINE_STYLE},
      {"-khtml-box-lines", CSS_PROP__KHTML_BOX_LINES},
      {"-khtml-margin-bottom-collapse", CSS_PROP__KHTML_MARGIN_BOTTOM_COLLAPSE},
      {"outline-width", CSS_PROP_OUTLINE_WIDTH},
      {"-khtml-box-ordinal-group", CSS_PROP__KHTML_BOX_ORDINAL_GROUP},
      {"content", CSS_PROP_CONTENT},
      {"word-spacing", CSS_PROP_WORD_SPACING},
      {"font-style", CSS_PROP_FONT_STYLE},
      {"list-style-position", CSS_PROP_LIST_STYLE_POSITION},
      {"font-size-adjust", CSS_PROP_FONT_SIZE_ADJUST},
      {"text-overline-style", CSS_PROP_TEXT_OVERLINE_STYLE},
      {"text-overline-width", CSS_PROP_TEXT_OVERLINE_WIDTH},
      {"font-weight", CSS_PROP_FONT_WEIGHT},
      {"text-line-through-style", CSS_PROP_TEXT_LINE_THROUGH_STYLE},
      {"text-transform", CSS_PROP_TEXT_TRANSFORM},
      {"-khtml-binding", CSS_PROP__KHTML_BINDING},
      {"text-line-through-width", CSS_PROP_TEXT_LINE_THROUGH_WIDTH},
      {"outline-offset", CSS_PROP_OUTLINE_OFFSET},
      {"text-indent", CSS_PROP_TEXT_INDENT},
      {"font-family", CSS_PROP_FONT_FAMILY},
      {"text-underline", CSS_PROP_TEXT_UNDERLINE},
      {"text-underline-color", CSS_PROP_TEXT_UNDERLINE_COLOR},
      {"background-attachment", CSS_PROP_BACKGROUND_ATTACHMENT},
      {"text-underline-mode", CSS_PROP_TEXT_UNDERLINE_MODE},
      {"background-position", CSS_PROP_BACKGROUND_POSITION},
      {"-khtml-marquee-increment", CSS_PROP__KHTML_MARQUEE_INCREMENT},
      {"-khtml-border-horizontal-spacing", CSS_PROP__KHTML_BORDER_HORIZONTAL_SPACING},
      {"font-variant", CSS_PROP_FONT_VARIANT},
      {"background-position-x", CSS_PROP_BACKGROUND_POSITION_X},
      {"background-position-y", CSS_PROP_BACKGROUND_POSITION_Y},
      {"text-underline-style", CSS_PROP_TEXT_UNDERLINE_STYLE},
      {"text-underline-width", CSS_PROP_TEXT_UNDERLINE_WIDTH},
      {"counter-increment", CSS_PROP_COUNTER_INCREMENT}
    };

  static const short lookup[] =
    {
       -1,  -1,  -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,   1,   2,  -1,  -1,  -1,
       -1,  -1,  -1,   3,   4,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        5,   6,  -1,  -1,  -1,  -1,  -1,   7,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,   8,  -1,  -1,
       -1,  -1,  -1,   9,  -1,  10,  -1,  -1,  -1,  11,
       -1,  12,  -1,  -1,  -1,  -1,  13,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  14,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  15,  -1,
       -1,  -1,  -1,  -1,  16,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  17,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  18,  -1,
       19,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       20,  -1,  -1,  -1,  -1,  -1,  21,  -1,  -1,  22,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  23,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  24,
       -1,  -1,  25,  -1,  -1,  26,  -1,  -1,  -1,  -1,
       27,  28,  -1,  -1,  -1,  -1,  29,  30,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  31,  -1,  -1,  32,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  33,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       34,  -1,  -1,  -1,  -1,  -1,  -1,  35,  36,  -1,
       -1,  37,  -1,  -1,  -1,  -1,  38,  39,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  40,  -1,  -1,
       41,  -1,  -1,  42,  -1,  43,  -1,  -1,  -1,  -1,
       -1,  44,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       45,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  46,
       -1,  -1,  -1,  -1,  -1,  -1,  47,  48,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  49,  50,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  51,
       -1,  52,  -1,  -1,  -1,  -1,  53,  -1,  -1,  -1,
       -1,  54,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  55,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  56,  -1,  -1,
       -1,  57,  58,  59,  -1,  -1,  -1,  -1,  -1,  -1,
       60,  -1,  -1,  -1,  -1,  -1,  61,  -1,  62,  -1,
       -1,  -1,  63,  64,  -1,  -1,  65,  -1,  -1,  66,
       -1,  -1,  67,  -1,  -1,  -1,  -1,  68,  -1,  -1,
       69,  -1,  -1,  -1,  -1,  -1,  -1,  70,  71,  -1,
       -1,  -1,  -1,  72,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  73,
       -1,  -1,  -1,  -1,  -1,  74,  -1,  -1,  -1,  75,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  76,  -1,  -1,
       -1,  77,  78,  79,  -1,  80,  81,  82,  83,  -1,
       -1,  84,  -1,  -1,  85,  -1,  -1,  86,  -1,  -1,
       -1,  -1,  87,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  88,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  89,  -1,  -1,  -1,  -1,  -1,  90,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  91,
       92,  93,  -1,  -1,  94,  95,  -1,  96,  97,  -1,
       -1,  -1,  -1,  -1,  98,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  99,  -1,  -1,  -1,  -1,  -1, 100,  -1, 101,
       -1, 102, 103, 104,  -1, 105,  -1,  -1, 106,  -1,
       -1,  -1, 107,  -1,  -1, 108,  -1,  -1,  -1, 109,
      110,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 111, 112, 113,  -1,  -1,  -1, 114,  -1,  -1,
       -1,  -1, 115,  -1,  -1,  -1, 116,  -1, 117,  -1,
       -1,  -1,  -1,  -1,  -1,  -1, 118,  -1,  -1, 119,
       -1,  -1, 120,  -1,  -1,  -1, 121,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 122, 123,  -1,  -1, 124,  -1,
       -1,  -1,  -1,  -1,  -1,  -1, 125,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 126,
       -1,  -1,  -1, 127,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 128,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1, 129,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1, 130,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      131,  -1,  -1,  -1, 132,  -1, 133,  -1,  -1, 134,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 135,  -1, 136,  -1, 137,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1, 138,  -1,  -1,  -1,  -1, 139,
       -1,  -1,  -1, 140,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 141,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 142,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 143,  -1,  -1, 144,  -1,  -1,  -1,  -1,  -1,
      145,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 146,  -1,  -1, 147,  -1,  -1,  -1,  -1, 148,
       -1,  -1,  -1,  -1, 149,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1, 150,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1, 151,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 152,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1, 153,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1, 154,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      155,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1, 156
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash_prop (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist_prop[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist_prop[index];
            }
        }
    }
  return 0;
}
static const char * const propertyList[] = {
"",
"background-color", 
"background-image", 
"background-repeat", 
"background-attachment", 
"background-position", 
"background-position-x", 
"background-position-y", 
"-khtml-binding", 
"border-collapse", 
"border-spacing", 
"-khtml-border-horizontal-spacing", 
"-khtml-border-vertical-spacing", 
"border-top-color", 
"border-right-color", 
"border-bottom-color", 
"border-left-color", 
"border-top-style", 
"border-right-style", 
"border-bottom-style", 
"border-left-style", 
"border-top-width", 
"border-right-width", 
"border-bottom-width", 
"border-left-width", 
"bottom", 
"-khtml-box-align", 
"-khtml-box-direction", 
"-khtml-box-flex", 
"-khtml-box-flex-group", 
"-khtml-box-lines", 
"-khtml-box-ordinal-group", 
"-khtml-box-orient", 
"-khtml-box-pack", 
"caption-side", 
"clear", 
"clip", 
"color", 
"content", 
"counter-increment", 
"counter-reset", 
"cursor", 
"direction", 
"display", 
"empty-cells", 
"float", 
"font-family", 
"font-size", 
"font-size-adjust", 
"font-stretch", 
"font-style", 
"font-variant", 
"font-weight", 
"height", 
"left", 
"letter-spacing", 
"-apple-line-clamp", 
"line-height", 
"list-style-image", 
"list-style-position", 
"list-style-type", 
"margin-top", 
"margin-right", 
"margin-bottom", 
"margin-left", 
"-khtml-margin-collapse", 
"-khtml-margin-top-collapse", 
"-khtml-margin-bottom-collapse", 
"-khtml-margin-start", 
"-khtml-marquee", 
"-khtml-marquee-direction", 
"-khtml-marquee-increment", 
"-khtml-marquee-repetition", 
"-khtml-marquee-speed", 
"-khtml-marquee-style", 
"max-height", 
"max-width", 
"min-height", 
"min-width", 
"opacity", 
"orphans", 
"outline-color", 
"outline-offset", 
"outline-style", 
"outline-width", 
"overflow", 
"padding-top", 
"padding-right", 
"padding-bottom", 
"padding-left", 
"-khtml-padding-start", 
"page", 
"page-break-after", 
"page-break-before", 
"page-break-inside", 
"position", 
"quotes", 
"right", 
"size", 
"table-layout", 
"text-align", 
"text-decoration", 
"text-indent", 
"text-line-through", 
"text-line-through-color", 
"text-line-through-mode", 
"text-line-through-style", 
"text-line-through-width", 
"text-overflow", 
"text-overline", 
"text-overline-color", 
"text-overline-mode", 
"text-overline-style", 
"text-overline-width", 
"text-shadow", 
"text-transform", 
"text-underline", 
"text-underline-color", 
"text-underline-mode", 
"text-underline-style", 
"text-underline-width", 
"-apple-text-size-adjust", 
"-apple-dashboard-region", 
"top", 
"unicode-bidi", 
"-khtml-user-drag", 
"-khtml-user-modify", 
"-khtml-user-select", 
"vertical-align", 
"visibility", 
"white-space", 
"widows", 
"width", 
"word-wrap", 
"word-spacing", 
"z-index", 
"background", 
"border", 
"border-color", 
"border-style", 
"border-top", 
"border-right", 
"border-bottom", 
"border-left", 
"border-width", 
"font", 
"list-style", 
"margin", 
"outline", 
"padding", 
"scrollbar-face-color", 
"scrollbar-shadow-color", 
"scrollbar-highlight-color", 
"scrollbar-3dlight-color", 
"scrollbar-darkshadow-color", 
"scrollbar-track-color", 
"scrollbar-arrow-color", 
"-khtml-flow-mode", 
    0
};
DOMString getPropertyName(unsigned short id)
{
    if(id >= CSS_PROP_TOTAL || id == 0)
      return DOMString();
    else
      return DOMString(propertyList[id]);
};

