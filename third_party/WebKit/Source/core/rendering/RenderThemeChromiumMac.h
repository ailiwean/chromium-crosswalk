/*
 * This file is part of the theme implementation for form controls in WebCore.
 *
 * Copyright (C) 2005 Apple Computer, Inc.
 * Copyright (C) 2008, 2009 Google, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef RenderThemeChromiumMac_h
#define RenderThemeChromiumMac_h

#import "RenderTheme.h"
#import "RenderThemeChromiumCommon.h"

#import <wtf/HashMap.h>
#import <wtf/RetainPtr.h>

OBJC_CLASS WebCoreRenderThemeNotificationObserver;

namespace WebCore {

class RenderThemeChromiumMac : public RenderTheme {
public:
    static PassRefPtr<RenderTheme> create();

    virtual bool supportsDataListUI(const AtomicString& type) const OVERRIDE;

    // A method asking if the control changes its tint when the window has focus or not.
    virtual bool controlSupportsTints(const RenderObject*) const;

    // A general method asking if any control tinting is supported at all.
    virtual bool supportsControlTints() const { return true; }

    virtual void adjustRepaintRect(const RenderObject*, IntRect&) OVERRIDE;

    virtual bool isControlStyled(const RenderStyle*, const BorderData&, const FillLayer&, const Color& backgroundColor) const;

    virtual Color platformActiveSelectionBackgroundColor() const;
    virtual Color platformInactiveSelectionBackgroundColor() const;
    virtual Color platformActiveListBoxSelectionBackgroundColor() const;
    virtual Color platformActiveListBoxSelectionForegroundColor() const;
    virtual Color platformInactiveListBoxSelectionBackgroundColor() const;
    virtual Color platformInactiveListBoxSelectionForegroundColor() const;
    virtual Color platformFocusRingColor() const;

    virtual ScrollbarControlSize scrollbarControlSizeForPart(ControlPart) { return SmallScrollbar; }

    virtual void platformColorsDidChange();

    // System fonts.
    virtual void systemFont(int cssValueId, FontDescription&) const;

    virtual int minimumMenuListSize(RenderStyle*) const;

    virtual void adjustSliderThumbSize(RenderStyle*, Element*) const;

#if ENABLE(DATALIST_ELEMENT)
    virtual IntSize sliderTickSize() const OVERRIDE;
    virtual int sliderTickOffsetFromTrackCenter() const OVERRIDE;
#endif

    virtual int popupInternalPaddingLeft(RenderStyle*) const;
    virtual int popupInternalPaddingRight(RenderStyle*) const;
    virtual int popupInternalPaddingTop(RenderStyle*) const;
    virtual int popupInternalPaddingBottom(RenderStyle*) const;

    virtual bool paintCapsLockIndicator(RenderObject*, const PaintInfo&, const IntRect&) OVERRIDE;

    virtual bool popsMenuByArrowKeys() const OVERRIDE { return true; }

    virtual IntSize meterSizeForBounds(const RenderMeter*, const IntRect&) const OVERRIDE;
    virtual bool paintMeter(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool supportsMeter(ControlPart) const;

    // Returns the repeat interval of the animation for the progress bar.
    virtual double animationRepeatIntervalForProgressBar(RenderProgress*) const;
    // Returns the duration of the animation for the progress bar.
    virtual double animationDurationForProgressBar(RenderProgress*) const;

    virtual Color systemColor(int cssValueId) const;

protected:
    RenderThemeChromiumMac();
    virtual ~RenderThemeChromiumMac();

    virtual bool supportsSelectionForegroundColors() const { return false; }

    virtual bool paintTextField(RenderObject*, const PaintInfo&, const IntRect&);
    virtual void adjustTextFieldStyle(StyleResolver*, RenderStyle*, Element*) const;

    virtual bool paintTextArea(RenderObject*, const PaintInfo&, const IntRect&);
    virtual void adjustTextAreaStyle(StyleResolver*, RenderStyle*, Element*) const;

    virtual bool paintMenuList(RenderObject*, const PaintInfo&, const IntRect&);
    virtual void adjustMenuListStyle(StyleResolver*, RenderStyle*, Element*) const;

    virtual bool paintMenuListButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual void adjustMenuListButtonStyle(StyleResolver*, RenderStyle*, Element*) const;

    virtual void adjustProgressBarStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintProgressBar(RenderObject*, const PaintInfo&, const IntRect&);

    virtual bool paintSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);

    virtual bool paintSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);

    virtual bool paintSearchField(RenderObject*, const PaintInfo&, const IntRect&);
    virtual void adjustSearchFieldStyle(StyleResolver*, RenderStyle*, Element*) const;

    virtual void adjustSearchFieldCancelButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldCancelButton(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldDecorationStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldDecoration(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldResultsDecorationStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldResultsDecoration(RenderObject*, const PaintInfo&, const IntRect&);

    virtual void adjustSearchFieldResultsButtonStyle(StyleResolver*, RenderStyle*, Element*) const;
    virtual bool paintSearchFieldResultsButton(RenderObject*, const PaintInfo&, const IntRect&);

#if ENABLE(VIDEO)
    virtual bool supportsClosedCaptioning() const { return true; }
#endif

private:
    virtual String fileListNameForWidth(const FileList*, const Font&, int width, bool multipleFilesAllowed) const OVERRIDE;

    IntRect inflateRect(const IntRect&, const IntSize&, const int* margins, float zoomLevel = 1.0f) const;

    FloatRect convertToPaintingRect(const RenderObject* inputRenderer, const RenderObject* partRenderer, const FloatRect& inputRect, const IntRect&) const;

    // Get the control size based off the font. Used by some of the controls (like buttons).
    NSControlSize controlSizeForFont(RenderStyle*) const;
    NSControlSize controlSizeForSystemFont(RenderStyle*) const;
    void setControlSize(NSCell*, const IntSize* sizes, const IntSize& minSize, float zoomLevel = 1.0f);
    void setSizeFromFont(RenderStyle*, const IntSize* sizes) const;
    IntSize sizeForFont(RenderStyle*, const IntSize* sizes) const;
    IntSize sizeForSystemFont(RenderStyle*, const IntSize* sizes) const;
    void setFontFromControlSize(StyleResolver*, RenderStyle*, NSControlSize) const;

    void updateCheckedState(NSCell*, const RenderObject*);
    void updateEnabledState(NSCell*, const RenderObject*);
    void updateFocusedState(NSCell*, const RenderObject*);
    void updatePressedState(NSCell*, const RenderObject*);

    // Helpers for adjusting appearance and for painting

    void setPopupButtonCellState(const RenderObject*, const IntRect&);
    const IntSize* popupButtonSizes() const;
    const int* popupButtonMargins() const;
    const int* popupButtonPadding(NSControlSize) const;
    void paintMenuListButtonGradients(RenderObject*, const PaintInfo&, const IntRect&);
    const IntSize* menuListSizes() const;

    const IntSize* searchFieldSizes() const;
    const IntSize* cancelButtonSizes() const;
    const IntSize* resultsButtonSizes() const;
    void setSearchCellState(RenderObject*, const IntRect&);
    void setSearchFieldSize(RenderStyle*) const;

    NSPopUpButtonCell* popupButton() const;
    NSSearchFieldCell* search() const;
    NSMenu* searchMenuTemplate() const;
    NSSliderCell* sliderThumbHorizontal() const;
    NSSliderCell* sliderThumbVertical() const;
    NSTextFieldCell* textField() const;

    NSLevelIndicatorStyle levelIndicatorStyleFor(ControlPart) const;
    NSLevelIndicatorCell* levelIndicatorFor(const RenderMeter*) const;

    int minimumProgressBarHeight(RenderStyle*) const;
    const IntSize* progressBarSizes() const;
    const int* progressBarMargins(NSControlSize) const;

protected:
#if ENABLE(VIDEO)
    virtual void adjustMediaSliderThumbSize(RenderStyle*) const;
    virtual bool paintMediaPlayButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaMuteButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);
    virtual String extraMediaControlsStyleSheet();
    virtual String extraFullScreenStyleSheet();
  
    virtual bool paintMediaSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderContainer(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaVolumeSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool usesMediaControlStatusDisplay() { return false; }
    virtual bool hasOwnDisabledStateHandlingFor(ControlPart) const { return true; }
    virtual bool usesVerticalVolumeSlider() const { return false; }
    virtual String formatMediaControlsTime(float time) const;
    virtual String formatMediaControlsCurrentTime(float currentTime, float duration) const;
    virtual String formatMediaControlsRemainingTime(float currentTime, float duration) const;
    virtual bool paintMediaFullscreenButton(RenderObject*, const PaintInfo&, const IntRect&);
    virtual bool paintMediaToggleClosedCaptionsButton(RenderObject*, const PaintInfo&, const IntRect&);
#endif

    // Controls color values returned from platformFocusRingColor(). systemColor() will be used when false.
    bool usesTestModeFocusRingColor() const;
    // A view associated to the contained document. Subclasses may not have such a view and return a fake.
    NSView* documentViewFor(RenderObject*) const;

private:
    virtual Color disabledTextColor(const Color& textColor, const Color&) const OVERRIDE { return textColor; }
    virtual void updateActiveState(NSCell*, const RenderObject*);
    virtual String extraDefaultStyleSheet();
#if ENABLE(DATALIST_ELEMENT)
    virtual LayoutUnit sliderTickSnappingThreshold() const OVERRIDE;
#endif
#if ENABLE(INPUT_MULTIPLE_FIELDS_UI)
    virtual bool supportsCalendarPicker(const AtomicString& type) const OVERRIDE;
#endif
    virtual bool shouldShowPlaceholderWhenFocused() const OVERRIDE;

    mutable RetainPtr<NSPopUpButtonCell> m_popupButton;
    mutable RetainPtr<NSSearchFieldCell> m_search;
    mutable RetainPtr<NSMenu> m_searchMenuTemplate;
    mutable RetainPtr<NSSliderCell> m_sliderThumbHorizontal;
    mutable RetainPtr<NSSliderCell> m_sliderThumbVertical;
    mutable RetainPtr<NSLevelIndicatorCell> m_levelIndicator;
    mutable RetainPtr<NSTextFieldCell> m_textField;

    bool m_isSliderThumbHorizontalPressed;
    bool m_isSliderThumbVerticalPressed;

    mutable HashMap<int, RGBA32> m_systemColorCache;

    RetainPtr<WebCoreRenderThemeNotificationObserver> m_notificationObserver;
};

} // namespace WebCore

#endif // RenderThemeChromiumMac_h
