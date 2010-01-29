/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
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
 */

#include "config.h"
#include "PainterOpenVG.h"

#include "Color.h"
#include "DashArray.h"
#include "FloatPoint.h"
#include "FloatQuad.h"
#include "FloatRect.h"
#include "IntRect.h"
#include "IntSize.h"
#include "NotImplemented.h"
#include "SurfaceOpenVG.h"
#include "TransformationMatrix.h"
#include "VGUtils.h"

#if PLATFORM(EGL)
#include "EGLUtils.h"
#endif

#include <vgu.h>

#include <wtf/Assertions.h>
#include <wtf/MathExtras.h>

namespace WebCore {

static bool isNonRotatedAffineTransformation(const TransformationMatrix& matrix)
{
    return matrix.m12() <= FLT_EPSILON && matrix.m13() <= FLT_EPSILON && matrix.m14() <= FLT_EPSILON
        && matrix.m21() <= FLT_EPSILON && matrix.m23() <= FLT_EPSILON && matrix.m24() <= FLT_EPSILON
        && matrix.m31() <= FLT_EPSILON && matrix.m32() <= FLT_EPSILON && matrix.m34() <= FLT_EPSILON
        && matrix.m44() >= 1 - FLT_EPSILON;
}

static VGCapStyle toVGCapStyle(LineCap lineCap)
{
    switch (lineCap) {
    case RoundCap:
        return VG_CAP_ROUND;
    case SquareCap:
        return VG_CAP_SQUARE;
    case ButtCap:
    default:
        return VG_CAP_BUTT;
    }
}

static VGJoinStyle toVGJoinStyle(LineJoin lineJoin)
{
    switch (lineJoin) {
    case RoundJoin:
        return VG_JOIN_ROUND;
    case BevelJoin:
        return VG_JOIN_BEVEL;
    case MiterJoin:
    default:
        return VG_JOIN_MITER;
    }
}

static VGFillRule toVGFillRule(WindRule fillRule)
{
    return fillRule == RULE_EVENODD ? VG_EVEN_ODD : VG_NON_ZERO;
}

static VGuint colorToVGColor(const Color& color)
{
    VGuint vgColor = color.red();
    vgColor = (vgColor << 8) | color.green();
    vgColor = (vgColor << 8) | color.blue();
    vgColor = (vgColor << 8) | color.alpha();
    return vgColor;
}

static void setVGSolidColor(VGPaintMode paintMode, const Color& color)
{
    VGPaint paint = vgCreatePaint();
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetColor(paint, colorToVGColor(color));
    vgSetPaint(paint, paintMode);
    vgDestroyPaint(paint);
    ASSERT_VG_NO_ERROR();
}


struct PlatformPainterState {
    TransformationMatrix surfaceTransformationMatrix;
    CompositeOperator compositeOperation;
    float opacity;

    bool scissoringEnabled;
    FloatRect scissorRect;

    Color fillColor;
    StrokeStyle strokeStyle;
    Color strokeColor;
    float strokeThickness;
    LineCap strokeLineCap;
    LineJoin strokeLineJoin;
    float strokeMiterLimit;
    DashArray strokeDashArray;
    float strokeDashOffset;

    bool antialiasingEnabled;

    PlatformPainterState()
        : compositeOperation(CompositeSourceOver)
        , opacity(1.0)
        , scissoringEnabled(false)
        , fillColor(Color::black)
        , strokeStyle(NoStroke)
        , strokeThickness(0.0)
        , strokeLineCap(ButtCap)
        , strokeLineJoin(MiterJoin)
        , strokeMiterLimit(4.0)
        , strokeDashOffset(0.0)
        , antialiasingEnabled(true)
    {
    }

    PlatformPainterState(const PlatformPainterState& state)
    {
        surfaceTransformationMatrix = state.surfaceTransformationMatrix;

        scissoringEnabled = state.scissoringEnabled;
        scissorRect = state.scissorRect;
        copyPaintState(&state);
    }

    void copyPaintState(const PlatformPainterState* other)
    {
        compositeOperation = other->compositeOperation;
        opacity = other->opacity;

        fillColor = other->fillColor;
        strokeStyle = other->strokeStyle;
        strokeColor = other->strokeColor;
        strokeThickness = other->strokeThickness;
        strokeLineCap = other->strokeLineCap;
        strokeLineJoin = other->strokeLineJoin;
        strokeMiterLimit = other->strokeMiterLimit;
        strokeDashArray = other->strokeDashArray;
        strokeDashOffset = other->strokeDashOffset;

        antialiasingEnabled = other->antialiasingEnabled;
    }

    void applyState(PainterOpenVG* painter)
    {
        ASSERT(painter);

        setVGSolidColor(VG_FILL_PATH, fillColor);
        setVGSolidColor(VG_STROKE_PATH, strokeColor);

        vgSetf(VG_STROKE_LINE_WIDTH, strokeThickness);
        vgSeti(VG_STROKE_CAP_STYLE, toVGCapStyle(strokeLineCap));
        vgSeti(VG_STROKE_JOIN_STYLE, toVGJoinStyle(strokeLineJoin));
        vgSetf(VG_STROKE_MITER_LIMIT, strokeMiterLimit);

        if (antialiasingEnabled)
            vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_FASTER);
        else
            vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);

        applyBlending(painter);
        applyStrokeStyle();

        applyTransformationMatrix(painter);
        applyScissorRect();
    }

    void applyBlending(PainterOpenVG* painter)
    {
        VGBlendMode blendMode = VG_BLEND_SRC_OVER;

        switch (compositeOperation) {
        case CompositeClear: {
            // Clear means "set to fully transparent regardless of SRC".
            // We implement that by multiplying DST with white color
            // (= no changes) and an alpha of 1.0 - opacity, so the destination
            // pixels will be fully transparent when opacity == 1.0 and
            // unchanged when opacity == 0.0.
            blendMode = VG_BLEND_DST_IN;
            const VGfloat values[] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 - opacity };
            vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);
            vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
            ASSERT_VG_NO_ERROR();
            break;
        }
        case CompositeCopy:
            blendMode = VG_BLEND_SRC;
            break;
        case CompositeSourceOver:
            blendMode = VG_BLEND_SRC_OVER;
            break;
        case CompositeSourceIn:
            blendMode = VG_BLEND_SRC_IN;
            break;
        case CompositeSourceOut:
            notImplemented();
            break;
        case CompositeSourceAtop:
            notImplemented();
            break;
        case CompositeDestinationOver:
            blendMode = VG_BLEND_DST_OVER;
            break;
        case CompositeDestinationIn:
            blendMode = VG_BLEND_DST_IN;
            break;
        case CompositeDestinationOut:
            notImplemented();
            break;
        case CompositeDestinationAtop:
            notImplemented();
            break;
        case CompositeXOR:
            notImplemented();
            break;
        case CompositePlusDarker:
            blendMode = VG_BLEND_DARKEN;
            break;
        case CompositeHighlight:
            notImplemented();
            break;
        case CompositePlusLighter:
            blendMode = VG_BLEND_LIGHTEN;
            break;
        }

        if (compositeOperation != CompositeClear) {
            if (opacity >= (1.0 - FLT_EPSILON))
                vgSeti(VG_COLOR_TRANSFORM, VG_FALSE);
            else if (blendMode == VG_BLEND_SRC) {
                blendMode = VG_BLEND_SRC_OVER;
                VGfloat values[] = { 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, opacity };
                vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);
                vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
            } else {
                VGfloat values[] = { 1.0, 1.0, 1.0, opacity, 0.0, 0.0, 0.0, 0.0 };
                vgSetfv(VG_COLOR_TRANSFORM_VALUES, 8, values);
                vgSeti(VG_COLOR_TRANSFORM, VG_TRUE);
            }
            ASSERT_VG_NO_ERROR();
        }

        vgSeti(VG_BLEND_MODE, blendMode);
        ASSERT_VG_NO_ERROR();
    }

    void applyTransformationMatrix(PainterOpenVG* painter)
    {
        // There are *five* separate transforms that can be applied to OpenVG as of 1.1
        // but it is not clear that we need to set them separately.  Instead we set them
        // all right here and let this be a call to essentially set the world transformation!
        VGMatrix vgMatrix(surfaceTransformationMatrix);
        const VGfloat* vgFloatArray = vgMatrix.toVGfloat();

        vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
        vgLoadMatrix(vgFloatArray);
        ASSERT_VG_NO_ERROR();

        vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
        vgLoadMatrix(vgFloatArray);
        ASSERT_VG_NO_ERROR();

#ifdef OPENVG_VERSION_1_1
        vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);
        vgLoadMatrix(vgFloatArray);
        ASSERT_VG_NO_ERROR();
#endif
    }

    void applyScissorRect()
    {
        if (scissoringEnabled) {
            vgSeti(VG_SCISSORING, VG_TRUE);
            vgSetfv(VG_SCISSOR_RECTS, 4, VGRect(scissorRect).toVGfloat());
        } else
            vgSeti(VG_SCISSORING, VG_FALSE);

        ASSERT_VG_NO_ERROR();
    }

    void applyStrokeStyle()
    {
        if (strokeStyle == DottedStroke) {
            VGfloat vgFloatArray[2] = { 1.0, 1.0 };
            vgSetfv(VG_STROKE_DASH_PATTERN, 2, vgFloatArray);
            vgSetf(VG_STROKE_DASH_PHASE, 0.0);
        } else if (strokeStyle == DashedStroke) {
            if (!strokeDashArray.size()) {
                VGfloat vgFloatArray[2] = { 4.0, 3.0 };
                vgSetfv(VG_STROKE_DASH_PATTERN, 2, vgFloatArray);
            } else {
                Vector<VGfloat> vgFloatArray(strokeDashArray.size());
                for (int i = 0; i < strokeDashArray.size(); ++i)
                    vgFloatArray[i] = strokeDashArray[i];

                vgSetfv(VG_STROKE_DASH_PATTERN, vgFloatArray.size(), vgFloatArray.data());
            }
            vgSetf(VG_STROKE_DASH_PHASE, strokeDashOffset);
        } else {
            vgSetfv(VG_STROKE_DASH_PATTERN, 0, 0);
            vgSetf(VG_STROKE_DASH_PHASE, 0.0);
        }

        ASSERT_VG_NO_ERROR();
    }

    inline bool strokeDisabled() const
    {
        return (compositeOperation == CompositeSourceOver
            && (strokeStyle == NoStroke || !strokeColor.alpha()));
    }

    inline bool fillDisabled() const
    {
        return (compositeOperation == CompositeSourceOver && !fillColor.alpha());
    }
};


PainterOpenVG::PainterOpenVG()
    : m_state(0)
    , m_surface(0)
{
}

PainterOpenVG::PainterOpenVG(SurfaceOpenVG* surface)
    : m_state(0)
    , m_surface(0)
{
    ASSERT(surface);
    begin(surface);
}

PainterOpenVG::~PainterOpenVG()
{
    end();
}

void PainterOpenVG::begin(SurfaceOpenVG* surface)
{
    if (surface == m_surface)
        return;

    ASSERT(surface);
    ASSERT(!m_state);

    m_surface = surface;

    m_stateStack.append(new PlatformPainterState());
    m_state = m_stateStack.last();

    m_surface->setActivePainter(this);
    m_surface->makeCurrent();
}

void PainterOpenVG::end()
{
    if (!m_surface)
        return;

    m_surface->setActivePainter(0);
    m_surface = 0;

    destroyPainterStates();
}

void PainterOpenVG::destroyPainterStates()
{
    PlatformPainterState* state = 0;
    while (!m_stateStack.isEmpty()) {
        state = m_stateStack.last();
        m_stateStack.removeLast();
        delete state;
    }
    m_state = 0;
}

// Called by friend SurfaceOpenVG, private otherwise.
void PainterOpenVG::applyState()
{
    ASSERT(m_state);
    m_state->applyState(this);
}

/**
 * Copy the current back buffer image onto the surface.
 *
 * Call this method when all painting operations have been completed,
 * otherwise the surface won't visibly change.
 */
void PainterOpenVG::blitToSurface()
{
    ASSERT(m_state); // implies m_surface
    m_surface->flush();
}

TransformationMatrix PainterOpenVG::transformationMatrix() const
{
    ASSERT(m_state);
    return m_state->surfaceTransformationMatrix;
}

void PainterOpenVG::concatTransformationMatrix(const TransformationMatrix& matrix)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    // We do the multiplication ourself using WebCore's TransformationMatrix rather than
    // offloading this to VG via vgMultMatrix to keep things simple and so we can maintain
    // state ourselves.
    m_state->surfaceTransformationMatrix.multLeft(matrix);
    m_state->applyTransformationMatrix(this);
}

void PainterOpenVG::setTransformationMatrix(const TransformationMatrix& matrix)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->surfaceTransformationMatrix = matrix;
    m_state->applyTransformationMatrix(this);
}

CompositeOperator PainterOpenVG::compositeOperation() const
{
    ASSERT(m_state);
    return m_state->compositeOperation;
}

void PainterOpenVG::setCompositeOperation(CompositeOperator op)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->compositeOperation = op;
    m_state->applyBlending(this);
}

float PainterOpenVG::opacity() const
{
    ASSERT(m_state);
    return m_state->opacity;
}

void PainterOpenVG::setOpacity(float opacity)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->opacity = opacity;
    m_state->applyBlending(this);
}

float PainterOpenVG::strokeThickness() const
{
    ASSERT(m_state);
    return m_state->strokeThickness;
}

void PainterOpenVG::setStrokeThickness(float thickness)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeThickness = thickness;
    vgSetf(VG_STROKE_LINE_WIDTH, thickness);
    ASSERT_VG_NO_ERROR();
}

StrokeStyle PainterOpenVG::strokeStyle() const
{
    ASSERT(m_state);
    return m_state->strokeStyle;
}

void PainterOpenVG::setStrokeStyle(const StrokeStyle& style)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeStyle = style;
    m_state->applyStrokeStyle();
}

void PainterOpenVG::setLineDash(const DashArray& dashArray, float dashOffset)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeDashArray = dashArray;
    m_state->strokeDashOffset = dashOffset;
    m_state->applyStrokeStyle();
}

void PainterOpenVG::setLineCap(LineCap lineCap)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeLineCap = lineCap;
    vgSeti(VG_STROKE_CAP_STYLE, toVGCapStyle(lineCap));
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::setLineJoin(LineJoin lineJoin)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeLineJoin = lineJoin;
    vgSeti(VG_STROKE_JOIN_STYLE, toVGJoinStyle(lineJoin));
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::setMiterLimit(float miterLimit)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeMiterLimit = miterLimit;
    vgSetf(VG_STROKE_MITER_LIMIT, miterLimit);
    ASSERT_VG_NO_ERROR();
}

Color PainterOpenVG::strokeColor() const
{
    ASSERT(m_state);
    return m_state->strokeColor;
}

void PainterOpenVG::setStrokeColor(const Color& color)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->strokeColor = color;
    setVGSolidColor(VG_STROKE_PATH, color);
}

Color PainterOpenVG::fillColor() const
{
    ASSERT(m_state);
    return m_state->fillColor;
}

void PainterOpenVG::setFillColor(const Color& color)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->fillColor = color;
    setVGSolidColor(VG_FILL_PATH, color);
}

bool PainterOpenVG::antialiasingEnabled() const
{
    ASSERT(m_state);
    return m_state->antialiasingEnabled;
}

void PainterOpenVG::setAntialiasingEnabled(bool enabled)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    m_state->antialiasingEnabled = enabled;

    if (enabled)
        vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_FASTER);
    else
        vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
}

void PainterOpenVG::scale(const FloatSize& scaleFactors)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    TransformationMatrix matrix = m_state->surfaceTransformationMatrix;
    matrix.scaleNonUniform(scaleFactors.width(), scaleFactors.height());
    setTransformationMatrix(matrix);
}

void PainterOpenVG::rotate(float radians)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    TransformationMatrix matrix = m_state->surfaceTransformationMatrix;
    matrix.rotate(rad2deg(radians));
    setTransformationMatrix(matrix);
}

void PainterOpenVG::translate(float dx, float dy)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    TransformationMatrix matrix = m_state->surfaceTransformationMatrix;
    matrix.translate(dx, dy);
    setTransformationMatrix(matrix);
}

void PainterOpenVG::intersectScissorRect(const FloatRect& rect)
{
    // Scissor rectangles are defined by float values, but e.g. painting
    // something red to a float-clipped rectangle and then painting something
    // white to the same rectangle will leave some red remnants as it is
    // rendered to full pixels in between. Also, some OpenVG implementations
    // are likely to clip to integer coordinates anyways because of the above
    // effect. So considering the above (and confirming through tests) the
    // visual result is better if we clip to the enclosing integer rectangle
    // rather than the exact float rectangle for scissoring.
    if (m_state->scissoringEnabled)
        m_state->scissorRect.intersect(FloatRect(enclosingIntRect(rect)));
    else {
        m_state->scissoringEnabled = true;
        m_state->scissorRect = FloatRect(enclosingIntRect(rect));
    }

    m_state->applyScissorRect();
}

void PainterOpenVG::intersectClipRect(const FloatRect& rect)
{
    ASSERT(m_state);
    m_surface->makeCurrent();

    if (m_state->surfaceTransformationMatrix.isIdentity()) {
        // No transformation required, skip all the complex stuff.
        intersectScissorRect(rect);
        return;
    }

    // Check if the actual destination rectangle is still rectilinear (can be
    // represented as FloatRect) so we could apply scissoring instead of
    // (potentially more expensive) path clipping. Note that scissoring is not
    // subject to transformations, so we need to do the transformation to
    // surface coordinates by ourselves.
    FloatQuad effectiveScissorQuad =
        m_state->surfaceTransformationMatrix.mapQuad(FloatQuad(rect));

    if (effectiveScissorQuad.isRectilinear())
        intersectScissorRect(effectiveScissorQuad.boundingBox());
    else {
        // The transformed scissorRect cannot be represented as FloatRect
        // anymore, so we need to perform masking instead. Not yet implemented.
        notImplemented();
    }
}

void PainterOpenVG::drawRect(const FloatRect& rect, VGbitfield specifiedPaintModes)
{
    ASSERT(m_state);

    VGbitfield paintModes = 0;
    if (!m_state->strokeDisabled())
        paintModes |= VG_STROKE_PATH;
    if (!m_state->fillDisabled())
        paintModes |= VG_FILL_PATH;

    paintModes &= specifiedPaintModes;

    if (!paintModes)
        return;

    m_surface->makeCurrent();

    VGPath path = vgCreatePath(
        VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
        1.0 /* scale */, 0.0 /* bias */,
        5 /* expected number of segments */,
        5 /* expected number of total coordinates */,
        VG_PATH_CAPABILITY_APPEND_TO
    );
    ASSERT_VG_NO_ERROR();

    if (vguRect(path, rect.x(), rect.y(), rect.width(), rect.height()) == VGU_NO_ERROR) {
        vgDrawPath(path, paintModes);
        ASSERT_VG_NO_ERROR();
    }

    vgDestroyPath(path);
    ASSERT_VG_NO_ERROR();
}

void PainterOpenVG::save(PainterOpenVG::SaveMode saveMode)
{
    ASSERT(m_state);

    // If the underlying context/surface was switched away by someone without
    // telling us, it might not correspond to the one assigned to this painter.
    // Switch back so we can save the state properly. (Should happen rarely.)
    // Use DontSaveOrApplyPainterState mode in order to avoid recursion.
    m_surface->makeCurrent(SurfaceOpenVG::DontSaveOrApplyPainterState);

    if (saveMode == PainterOpenVG::CreateNewState) {
        PlatformPainterState* state = new PlatformPainterState(*m_state);
        m_stateStack.append(state);
        m_state = m_stateStack.last();
    } else { // if (saveMode == PainterOpenVG::CreateNewStateWithPaintStateOnly) {
        PlatformPainterState* state = new PlatformPainterState();
        state->copyPaintState(m_state);
        m_stateStack.append(state);
        m_state = m_stateStack.last();
    }
}

void PainterOpenVG::restore()
{
    ASSERT(m_stateStack.size() >= 2);
    m_surface->makeCurrent(SurfaceOpenVG::DontApplyPainterState);

    PlatformPainterState* state = m_stateStack.last();
    m_stateStack.removeLast();
    delete state;

    m_state = m_stateStack.last();
    m_state->applyState(this);
}

}
