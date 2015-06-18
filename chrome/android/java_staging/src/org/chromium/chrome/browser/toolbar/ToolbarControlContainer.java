// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.toolbar;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.PorterDuff;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.graphics.drawable.ScaleDrawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;

import org.chromium.chrome.R;
import org.chromium.chrome.browser.compositor.layouts.eventfilter.EdgeSwipeHandler;
import org.chromium.chrome.browser.contextualsearch.SwipeRecognizer;
import org.chromium.chrome.browser.widget.ControlContainer;
import org.chromium.chrome.browser.widget.SmoothProgressBar;
import org.chromium.chrome.browser.widget.SmoothProgressBar.ProgressChangeListener;
import org.chromium.chrome.browser.widget.ViewResourceFrameLayout;
import org.chromium.ui.UiUtils;
import org.chromium.ui.resources.dynamics.ViewResourceAdapter;

/**
 * Layout for the browser controls (omnibox, menu, tab strip, etc..).
 */
public class ToolbarControlContainer extends FrameLayout implements ControlContainer {
    private final float mTabStripHeight;

    private Toolbar mToolbar;
    private ToolbarViewResourceFrameLayout mToolbarContainer;
    private View mMenuBtn;

    private final SwipeRecognizer mSwipeRecognizer;
    private EdgeSwipeHandler mSwipeHandler;

    private ViewResourceAdapter mProgressResourceAdapter;

    /**
     * Constructs a new control container.
     * <p>
     * This constructor is used when inflating from XML.
     *
     * @param context The context used to build this view.
     * @param attrs The attributes used to determine how to construct this view.
     */
    public ToolbarControlContainer(Context context, AttributeSet attrs) {
        super(context, attrs);
        mTabStripHeight = context.getResources().getDimension(R.dimen.tab_strip_height);
        mSwipeRecognizer = new SwipeRecognizerImpl(context);
    }

    @Override
    public ViewResourceAdapter getProgressResourceAdapter() {
        return mProgressResourceAdapter;
    }

    @Override
    public ViewResourceAdapter getToolbarResourceAdapter() {
        return mToolbarContainer.getResourceAdapter();
    }

    @Override
    public void setSwipeHandler(EdgeSwipeHandler handler) {
        mSwipeHandler = handler;
        mSwipeRecognizer.setSwipeHandler(handler);
    }

    @Override
    public void onFinishInflate() {
        mToolbar = (Toolbar) findViewById(R.id.toolbar);
        mToolbarContainer = (ToolbarViewResourceFrameLayout) findViewById(R.id.toolbar_container);
        mMenuBtn = findViewById(R.id.menu_button);

        // TODO(yusufo): Get rid of the calls below and avoid casting to the layout without making
        // the interface bigger.
        SmoothProgressBar progressView = ((ToolbarLayout) mToolbar).getProgressBar();
        if (progressView != null) {
            mProgressResourceAdapter = new ProgressViewResourceAdapter(progressView);
        }

        if (mToolbar instanceof ToolbarTablet) {
            // On tablet, draw a fake tab strip and toolbar until the compositor is ready to draw
            // the real tab strip. (On phone, the toolbar is made entirely of Android views, which
            // are already initialized.)
            setBackgroundResource(R.drawable.toolbar_background);
        }

        assert mToolbar != null;
        assert mMenuBtn != null;

        super.onFinishInflate();
    }

    /**
     * Invalidate the entire capturing bitmap region.
     */
    public void invalidateBitmap() {
        ((ToolbarViewResourceAdapter) getToolbarResourceAdapter()).forceInvalidate();
    }

    /**
     * Update whether the control container is ready to have the bitmap representation of
     * itself be captured.
     */
    public void setReadyForBitmapCapture(boolean ready) {
        mToolbarContainer.mReadyForBitmapCapture = ready;
    }

    /**
     * The layout that handles generating the toolbar view resource.
     */
    // Only publicly visible due to lint warnings.
    public static class ToolbarViewResourceFrameLayout extends ViewResourceFrameLayout {
        private boolean mReadyForBitmapCapture;

        public ToolbarViewResourceFrameLayout(Context context, AttributeSet attrs) {
            super(context, attrs);
        }

        @Override
        protected ViewResourceAdapter createResourceAdapter() {
            return new ToolbarViewResourceAdapter(
                    this, (Toolbar) findViewById(R.id.toolbar));
        }

        @Override
        protected boolean isReadyForCapture() {
            return mReadyForBitmapCapture;
        }
    }

    private static class ProgressViewResourceAdapter extends ViewResourceAdapter
            implements ProgressChangeListener {

        private final SmoothProgressBar mProgressView;
        private final Rect mPreviousDrawBounds = new Rect();
        private int mProgressVisibility;
        private int mProgress;

        ProgressViewResourceAdapter(SmoothProgressBar progressView) {
            super(progressView);

            mProgressView = progressView;
            mProgressVisibility = mProgressView.getVisibility();
            progressView.addProgressChangeListener(this);
        }

        @Override
        public void onProgressChanged(int progress) {
            if (mProgressVisibility != View.VISIBLE) return;
            if (progress < mProgress) {
                mPreviousDrawBounds.setEmpty();
            }
            mProgress = progress;
            invalidate(null);
        }

        @Override
        public void onProgressVisibilityChanged(int visibility) {
            if (mProgressVisibility == visibility) return;

            if (visibility == View.VISIBLE || mProgressVisibility == View.VISIBLE) {
                invalidate(null);
                mPreviousDrawBounds.setEmpty();
            }
            mProgressVisibility = visibility;
        }

        @Override
        protected void onCaptureStart(Canvas canvas, Rect dirtyRect) {
            canvas.save();
            canvas.clipRect(
                    mPreviousDrawBounds.right, 0,
                    mProgressView.getWidth(), mProgressView.getHeight());
            canvas.drawColor(0, PorterDuff.Mode.CLEAR);
            canvas.restore();

            super.onCaptureStart(canvas, dirtyRect);
        }

        @Override
        protected void capture(Canvas canvas) {
            if (mProgressVisibility != View.VISIBLE) {
                canvas.drawColor(0, PorterDuff.Mode.CLEAR);
            } else {
                super.capture(canvas);
            }
        }

        @Override
        protected void onCaptureEnd() {
            super.onCaptureEnd();
            // If we are unable to get accurate draw bounds, then set the draw bounds to
            // ensure the entire view is cleared.
            mPreviousDrawBounds.setEmpty();

            // The secondary drawable has an alpha component, so track the bounds of the
            // primary drawable.  This will allow the subsequent draw call to clear the secondary
            // portion not overlapped by the primary to prevent the alpha components from
            // stacking and getting progressively darker.
            Drawable progressDrawable = mProgressView.getProgressDrawable();
            if (progressDrawable instanceof LayerDrawable) {
                LayerDrawable progressLayerDrawable = (LayerDrawable) progressDrawable;
                for (int i = 0; i < progressLayerDrawable.getNumberOfLayers(); i++) {
                    if (progressLayerDrawable.getId(i) != android.R.id.progress) continue;
                    Drawable primaryProgressDrawable = progressLayerDrawable.getDrawable(i);
                    if (!(primaryProgressDrawable instanceof ScaleDrawable)) continue;

                    ((ScaleDrawable) primaryProgressDrawable).getDrawable().copyBounds(
                            mPreviousDrawBounds);
                }
            }
        }

        @Override
        protected void computeContentPadding(Rect outContentPadding) {
            super.computeContentPadding(outContentPadding);
            MarginLayoutParams layoutParams =
                    (MarginLayoutParams) mProgressView.getLayoutParams();
            outContentPadding.offset(0, layoutParams.topMargin);
        }
    }

    private static class ToolbarViewResourceAdapter extends ViewResourceAdapter {
        private final int mToolbarActualHeightPx;
        private final int[] mTempPosition = new int[2];

        private final View mToolbarContainer;
        private final Toolbar mToolbar;

        /** Builds the resource adapter for the toolbar. */
        public ToolbarViewResourceAdapter(View toolbarContainer, Toolbar toolbar) {
            super(toolbarContainer);

            mToolbarContainer = toolbarContainer;
            mToolbar = toolbar;
            mToolbarActualHeightPx = toolbarContainer.getResources().getDimensionPixelSize(
                    R.dimen.control_container_height);
        }

        /**
         * Force this resource to be recaptured in full, ignoring the checks
         * {@link #invalidate(Rect)} does.
         */
        public void forceInvalidate() {
            super.invalidate(null);
        }

        @Override
        public boolean isDirty() {
            return mToolbar != null && mToolbar.isReadyForTextureCapture() && super.isDirty();
        }

        @Override
        protected void onCaptureStart(Canvas canvas, Rect dirtyRect) {
            // Erase the shadow component of the bitmap if the clip rect included shadow.  Because
            // this region is not opaque painting twice would be bad.
            if (dirtyRect.intersects(
                    0, mToolbarActualHeightPx,
                    mToolbarContainer.getWidth(), mToolbarContainer.getHeight())) {
                canvas.save();
                canvas.clipRect(
                        0, mToolbarActualHeightPx,
                        mToolbarContainer.getWidth(), mToolbarContainer.getHeight());
                canvas.drawColor(0, PorterDuff.Mode.CLEAR);
                canvas.restore();
            }

            mToolbar.setTextureCaptureMode(true);

            super.onCaptureStart(canvas, dirtyRect);
        }

        @Override
        protected void onCaptureEnd() {
            mToolbar.setTextureCaptureMode(false);
        }

        @Override
        protected void computeContentPadding(Rect outContentPadding) {
            outContentPadding.set(0, 0, mToolbarContainer.getWidth(), mToolbarActualHeightPx);
        }

        @Override
        protected void computeContentAperture(Rect outContentAperture) {
            mToolbar.getLocationBarContentRect(outContentAperture);
            mToolbar.getPositionRelativeToContainer(mToolbarContainer, mTempPosition);
            outContentAperture.offset(mTempPosition[0], mTempPosition[1]);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        // Don't eat the event if we don't have a handler.
        if (mSwipeHandler == null) return false;

        // If we have ACTION_DOWN in this context, that means either no child consumed the event or
        // this class is the top UI at the event position. Then, we don't need to feed the event to
        // mGestureDetector here because the event is already once fed in onInterceptTouchEvent().
        // Moreover, we have to return true so that this class can continue to intercept all the
        // subsequent events.
        if (event.getActionMasked() == MotionEvent.ACTION_DOWN && !isOnTabStrip(event)) {
            return true;
        }

        return mSwipeRecognizer.onTouchEvent(event);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent event) {
        if (mSwipeHandler == null) return false;

        return mSwipeRecognizer.onTouchEvent(event);
    }

    private boolean isOnTabStrip(MotionEvent e) {
        return e.getY() <= mTabStripHeight;
    }

    private class SwipeRecognizerImpl extends SwipeRecognizer {
        public SwipeRecognizerImpl(Context context) {
            super(context);
        }

        @Override
        public boolean shouldRecognizeSwipe(MotionEvent e1, MotionEvent e2) {
            if (isOnTabStrip(e1)) return false;
            if (mToolbar.shouldIgnoreSwipeGesture()) return false;
            if (UiUtils.isKeyboardShowing(getContext(), ToolbarControlContainer.this)) return false;
            return true;
        }
    }
}
