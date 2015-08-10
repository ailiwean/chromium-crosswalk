// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/filters/vp9_raw_bits_reader.h"

#include <limits.h>

#include "base/logging.h"
#include "media/base/bit_reader.h"

namespace media {

Vp9RawBitsReader::Vp9RawBitsReader() : valid_(true) {}

Vp9RawBitsReader::~Vp9RawBitsReader() {}

void Vp9RawBitsReader::Initialize(const uint8_t* data, size_t size) {
  DCHECK(data);
  reader_.reset(new BitReader(data, size));
  valid_ = true;
}

int Vp9RawBitsReader::ReadBit() {
  DCHECK(reader_);
  int value = 0;
  valid_ = valid_ && reader_->ReadBits(1, &value);
  return value;
}

int Vp9RawBitsReader::ReadLiteral(int bits) {
  DCHECK(reader_);
  int value = 0;
  DCHECK_LT(static_cast<size_t>(bits), sizeof(value) * 8);
  valid_ = valid_ && reader_->ReadBits(bits, &value);
  return value;
}

int Vp9RawBitsReader::ReadSignedLiteral(int bits) {
  int value = ReadLiteral(bits);
  return ReadBit() ? -value : value;
}

size_t Vp9RawBitsReader::GetBytesRead() const {
  DCHECK(reader_);
  return (reader_->bits_read() + 7) / 8;
}

}  // namespace media
