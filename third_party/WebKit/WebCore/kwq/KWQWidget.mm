/*
 * Copyright (C) 2001 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include <qwidget.h>

#import "KWQView.h"

/*
    A QWidget rougly corresponds to an NSView.  In Qt a QFrame and QMainWindow inherit
    from a QWidget.  In Cocoa a NSWindow does not inherit from NSView.  We will
    emulate QWidgets using NSViews.
    
    The only QWidget emulated should be KHTMLView.  It's inheritance graph is
        KHTMLView
            QScrollView
                QFrame
                    QWidget
    
    We may not need to emulate any of the QScrollView behaviour as we MAY
    get scrolling behaviour for free simply be implementing all rendering 
    in an NSView and using NSScrollView.
    
    It appears that the only dependence of the window in khtml and kjs is
    the javascript functions that manipulate a window.  We may want to
    rework work that code rather than add NSWindow glue code.
*/

QWidget::QWidget(QWidget *parent=0, const char *name=0, WFlags f=0) 
{
    _initialize();
}


void QWidget::_initialize()
{
    data = (struct KWQWidgetData *)calloc (1, sizeof (struct KWQWidgetData));
    data->view = [[KWQView alloc] initWithFrame: NSMakeRect (0,0,0,0) widget: this];
}



QWidget::~QWidget() 
{
    [data->view release];

    // What about:
    // 	data->style
    // 	data->font
    // 	data->cursor
    
    free (data);
}


QSize QWidget::sizeHint() const 
{
}

void QWidget::resize(int w, int h) 
{
    internalSetGeometry( data->rect.x(), data->rect.y(), w, h, FALSE );
}

void QWidget::setActiveWindow() 
{
    // Map onto [NSView window] equivalent.
}


void QWidget::setEnabled(bool) 
{
    // Not relevant.
}


void QWidget::setAutoMask(bool) 
{
    // Not relevant. ??
}


void QWidget::setMouseTracking(bool) 
{
    // Not relevant. ??
}


int QWidget::winId() const
{
/*
    Does this winId need to be valid across sessions?  It appears to
    be used by HTMLDocument::setCookie( const DOMString & value ).  If
    so we have to rethink the current approach.
    
    Just return the hash of the NSView.
*/
    return (int)[data->view hash];
}


int QWidget::x() const 
{
    return data->rect.x();
}


int QWidget::y() const 
{
    return data->rect.y();
}


int QWidget::width() const 
{ 
    return data->rect.width();
}


int QWidget::height() const 
{
    return data->rect.height();
}


QSize QWidget::size() const 
{
    return data->rect.size();
}


void QWidget::resize(const QSize &s) 
{
    resize( s.width(), s.height());
}


QPoint QWidget::pos() const 
{
    return data->pos;
}


void QWidget::move(int x, int y) 
{
    internalSetGeometry( x + data->rect.x(),
			 y + data->rect.y(),
			 width(), height(), TRUE );
}


void QWidget::move(const QPoint &p) 
{
    move( p.x(), p.y() );
}


QWidget *QWidget::topLevelWidget() const 
{
    // This is only used by JavaScript to implement the various
    // window geometry manipulations and accessors, i.e.:
    // window.moveTo(), window.moveBy(), window.resizeBy(), window.resizeTo(),
    // outerWidth, outerHeight.
    
    // This should return a subclass of QWidget that fronts for an
    // NSWindow.
    return (QWidget *)this;
}


QPoint QWidget::mapToGlobal(const QPoint &p) const
{
    // This is only used by JavaScript to implement the getting
    // the screenX and screen Y coordinates.
    NSPoint sp;
    sp = [[data->view window] convertBaseToScreen: [data->view convertPoint: NSMakePoint ((float)p.x(), (float)p.y()) toView: nil]];
    return QPoint (sp.x, sp.y);
}


void QWidget::setFocus()
{
    // TBD
}


void QWidget::clearFocus()
{
    // TBD
}


QWidget::FocusPolicy QWidget::focusPolicy() const
{
    return data->focusPolicy;
}


void QWidget::setFocusPolicy(FocusPolicy fp)
{
    data->focusPolicy = fp;
}


void QWidget::setFocusProxy( QWidget * )
{
    // TBD
}


const QPalette& QWidget::palette() const
{
    return data->pal;
}


void QWidget::setPalette(const QPalette &palette)
{
    // FIXME!  What do we do about releasing the old palette?
    data->pal = palette;
}

void QWidget::unsetPalette()
{
    // Only called by RenderFormElement::layout, which I suspect will
    // have to be rewritten.  Do nothing.
}


QStyle &QWidget::style() const
{
    return *data->style;
}


void QWidget::setStyle(QStyle *style)
{
    // According to the Qt implementation 
    /*
    Sets the widget's GUI style to \a style. Ownership of the style
    object is not transferred.
    */
    data->style = style;
}


QFont QWidget::font() const
{
}


void QWidget::setFont(const QFont &font)
{
    // FIXME Not clear what we should do with old font!
    data->font = new QFont (font);
}


void QWidget::constPolish() const
{
    // Is this necessary?
}


QSize QWidget::minimumSizeHint() const
{
    // Used by embedded java (KJavaEmbed::sizeHint().  Will be replaced.
    // Used by RenderSubmitButton::calcMinMaxWidth(), where it is called
    // on a button widget.  Will be replaced.
    NSLog (@"ERROR %s:%s:%d (NOT IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
    return QSize (0,0);
}


bool QWidget::isVisible() const
{
    return [[data->view window] isVisible];
}


void QWidget::setCursor(const QCursor &cur)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
    if (data->cursor)
        delete data->cursor;
    data->cursor = new QCursor (cur);
}


bool QWidget::event(QEvent *)
{
    // This will eventually not be called, or called from our implementation
    // of run loop, or something???
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
    return TRUE;
}


bool QWidget::focusNextPrevChild(bool)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
    return TRUE;
}


bool QWidget::hasMouseTracking() const
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
    return true;
}


void QWidget::show()
{
    [[data->view window] deminiaturize: data->view];
}


void QWidget::hide()
{
    [[data->view window] miniaturize: data->view];
}


// no copying or assignment
// note that these are "standard" (no pendantic stuff needed)



// Non-public
void QWidget::setCRect( const QRect &r )
{
    // One rect is both the same.
    data->pos = r.topLeft();
    data->rect = r;
}

void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;

    QRect  r( x, y, w, h );

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if ( data->rect == r )
	return;

    QSize oldSize( size() );

    setCRect( r );

    bool isResize = size() != oldSize;

    // FIXME!  First approximation.  May need to translate coordinates.
    [data->view setFrame: NSMakeRect (data->rect.x(), data->rect.y(), data->rect.width(), data->rect.height())];
}

void QWidget::showEvent(QShowEvent *)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::hideEvent(QHideEvent *)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::wheelEvent(QWheelEvent *)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::keyPressEvent(QKeyEvent *)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::keyReleaseEvent(QKeyEvent *)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::focusOutEvent(QFocusEvent *)
{
    NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::setBackgroundMode(BackgroundMode)
{
     NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::setAcceptDrops(bool)
{
     NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


void QWidget::erase()
{
     NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


QWidget *QWidget::focusWidget() const
{
     NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}


QPoint QWidget::mapFromGlobal(const QPoint &) const
{
     NSLog (@"WARNING %s:%s:%d (NOT YET IMPLEMENTED)\n", __FILE__, __FUNCTION__, __LINE__);
}



#ifdef _KWQ_

void QWidget::paint (void *)
{
}

#if (defined(__APPLE__) && defined(__OBJC__) && defined(__cplusplus))
NSView *QWidget::getView()
{
    return data->view;
}
#else
void *QWidget::getView()
{
    return data->view;
}
#endif

#endif _KWQ_

