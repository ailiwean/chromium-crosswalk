// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_BUFFER_FORMAT_UTIL_H_
#define UI_GFX_BUFFER_FORMAT_UTIL_H_

#include "base/basictypes.h"
#include "ui/gfx/buffer_types.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/gfx_export.h"

namespace gfx {

// Returns the number of planes for |format|.
GFX_EXPORT size_t NumberOfPlanesForBufferFormat(BufferFormat format);

// Returns the subsampling factor applied to the given zero-indexed |plane| of
// |format| both horizontally and vertically.
GFX_EXPORT size_t SubsamplingFactorForBufferFormat(
    BufferFormat format, int plane);

// Returns the number of bytes used to store a row of the given zero-indexed
// |plane| of |format|.
GFX_EXPORT size_t RowSizeForBufferFormat(
    size_t width, gfx::BufferFormat format, int plane);
GFX_EXPORT bool RowSizeForBufferFormatChecked(
    size_t width, gfx::BufferFormat format, int plane, size_t* size_in_bytes)
    WARN_UNUSED_RESULT;

// Returns the number of bytes used to store all the planes of a given |format|.
GFX_EXPORT size_t BufferSizeForBufferFormat(
    const Size& size, gfx::BufferFormat format);
GFX_EXPORT bool BufferSizeForBufferFormatChecked(
    const Size& size, gfx::BufferFormat format, size_t* size_in_bytes)
    WARN_UNUSED_RESULT;

}  // namespace gfx

#endif  // UI_GFX_BUFFER_FORMAT_UTIL_H_
