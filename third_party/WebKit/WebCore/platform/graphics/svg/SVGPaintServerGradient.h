/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
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

#ifndef SVGPaintServerGradient_H
#define SVGPaintServerGradient_H

#ifdef SVG_SUPPORT

#include "AffineTransform.h"
#include "Color.h"
#include "SVGPaintServer.h"

#if PLATFORM(CG)
#include "SVGResourceImage.h"
#endif

#if PLATFORM(QT)
class QGradient;
#endif

namespace WebCore {

    enum SVGGradientSpreadMethod {
        SPREADMETHOD_PAD = 1,
        SPREADMETHOD_REPEAT = 2,
        SPREADMETHOD_REFLECT = 4
    };

    typedef std::pair<float, Color> SVGGradientStop;

    class SVGPaintServerGradient : public SVGPaintServer {
    public:
        SVGPaintServerGradient();
        virtual ~SVGPaintServerGradient();

        const Vector<SVGGradientStop>& gradientStops() const;
        void setGradientStops(const Vector<SVGGradientStop>&);
        void setGradientStops(SVGPaintServerGradient*);

        SVGGradientSpreadMethod spreadMethod() const;
        void setGradientSpreadMethod(const SVGGradientSpreadMethod&);

        // Gradient start and end points are percentages when used in boundingBox mode.
        // For instance start point with value (0,0) is top-left and end point with
        // value (100, 100) is bottom-right. BoundingBox mode is enabled by default.
        bool boundingBoxMode() const;
        void setBoundingBoxMode(bool mode = true);

        AffineTransform gradientTransform() const;
        void setGradientTransform(const AffineTransform&);

        SVGResourceListener* listener() const;
        void setListener(SVGResourceListener*);

        virtual TextStream& externalRepresentation(TextStream&) const;

#if PLATFORM(CG)
        virtual void teardown(KRenderingDeviceContext*, const RenderObject*, SVGPaintTargetType) const;
        virtual void renderPath(KRenderingDeviceContext*, const RenderPath*, SVGPaintTargetType) const;

        virtual bool setup(KRenderingDeviceContext*, const RenderObject*, SVGPaintTargetType) const;

        virtual void invalidate();

        // Helpers
        void invalidateCaches();
        void updateQuartzGradientStopsCache(const Vector<SVGGradientStop>&);
        void updateQuartzGradientCache(const SVGPaintServerGradient*);
#endif

#if PLATFORM(QT)
    protected:
        void fillColorArray(QGradient&, const Vector<SVGGradientStop>&, float opacity) const;
#endif

    private:
        Vector<SVGGradientStop> m_stops;
        SVGGradientSpreadMethod m_spreadMethod;
        bool m_boundingBoxMode;
        AffineTransform m_gradientTransform;
        SVGResourceListener* m_listener;

#if PLATFORM(CG)
    public:
        typedef struct {
            CGFloat colorArray[4];
            CGFloat offset;
            CGFloat previousDeltaInverse;
        } QuartzGradientStop;

        QuartzGradientStop* m_stopsCache;
        int m_stopsCount;

        CGShadingRef m_shadingCache;
        mutable RefPtr<SVGResourceImage> m_maskImage;
#endif
    };

    inline SVGGradientStop makeGradientStop(float offset, const Color& color)
    {
        return std::make_pair(offset, color);
    }

} // namespace WebCore

#endif

#endif // SVGPaintServerGradient_H
