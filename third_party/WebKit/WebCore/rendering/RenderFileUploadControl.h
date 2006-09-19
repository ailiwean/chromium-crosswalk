/*
 * Copyright (C) 2006 Apple Computer
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef RenderFileUploadControl_H
#define RenderFileUploadControl_H

#include "RenderBlock.h"

#include "FileChooser.h"
#include "HTMLInputElement.h"

namespace WebCore {

class HTMLFileUploadInnerButtonElement;
//class RenderFileUploadInnerFileBlock;
    
// RenderFileUploadControls contain a RenderButton (for opening the file chooser), and
// sufficient space to draw a file icon and filename. The RenderButton has an
// HTMLFileUploadInnerButtonElement shadow node associated with it to receive click/hover events.

class RenderFileUploadControl : public RenderBlock {
public:
    RenderFileUploadControl(Node*);
    ~RenderFileUploadControl();

    virtual void setStyle(RenderStyle*);
    virtual void updateFromElement();
    virtual void calcMinMaxWidth();
    virtual void paintObject(PaintInfo&, int tx, int ty);

    virtual void click(bool sendMouseEvents);

    void valueChanged();

    virtual const char* renderName() const { return "RenderFileUploadControl"; }

protected:
    int maxFilenameWidth();
    RenderStyle* createButtonStyle(RenderStyle* parentStyle = 0);
    
    RefPtr<HTMLFileUploadInnerButtonElement> m_button;
    FileChooser* m_fileChooser;
};

}

#endif
