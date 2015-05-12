// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/partial_circular_buffer.h"

#include <algorithm>

#include "base/logging.h"

namespace {

inline uint32 Min3(uint32 a, uint32 b, uint32 c) {
  return std::min(a, std::min(b, c));
}

}  // namespace

PartialCircularBuffer::PartialCircularBuffer(void* buffer,
                                             uint32 buffer_size)
    : buffer_data_(reinterpret_cast<BufferData*>(buffer)),
      memory_buffer_size_(buffer_size),
      data_size_(0),
      position_(0),
      total_read_(0) {
  uint32 header_size =
      buffer_data_->data - reinterpret_cast<uint8*>(buffer_data_);
  data_size_ = memory_buffer_size_ - header_size;

  DCHECK(buffer_data_);
  DCHECK_GE(memory_buffer_size_, header_size);
  DCHECK_LE(buffer_data_->total_written, data_size_);
  DCHECK_LT(buffer_data_->wrap_position, data_size_);
  DCHECK_LT(buffer_data_->end_position, data_size_);
}

PartialCircularBuffer::PartialCircularBuffer(void* buffer,
                                             uint32 buffer_size,
                                             uint32 wrap_position,
                                             bool append)
    : buffer_data_(reinterpret_cast<BufferData*>(buffer)),
      memory_buffer_size_(buffer_size),
      data_size_(0),
      position_(0),
      total_read_(0) {
  uint32 header_size =
      buffer_data_->data - reinterpret_cast<uint8*>(buffer_data_);
  data_size_ = memory_buffer_size_ - header_size;

  DCHECK(buffer_data_);
  DCHECK_GE(memory_buffer_size_, header_size);

  if (append) {
    DCHECK_LT(buffer_data_->wrap_position, data_size_);
    position_ = buffer_data_->end_position;
  } else {
    DCHECK_LT(wrap_position, data_size_);
    buffer_data_->total_written = 0;
    buffer_data_->wrap_position = wrap_position;
    buffer_data_->end_position = 0;
  }
}

uint32 PartialCircularBuffer::Read(void* buffer, uint32 buffer_size) {
  DCHECK(buffer_data_);
  if (total_read_ >= buffer_data_->total_written)
    return 0;

  uint8* buffer_uint8 = reinterpret_cast<uint8*>(buffer);
  uint32 read = 0;

  // Read from beginning part.
  if (position_ < buffer_data_->wrap_position) {
    uint32 to_wrap_pos = buffer_data_->wrap_position - position_;
    uint32 to_eow = buffer_data_->total_written - total_read_;
    uint32 to_read = Min3(buffer_size, to_wrap_pos, to_eow);
    memcpy(buffer_uint8, buffer_data_->data + position_, to_read);
    position_ += to_read;
    total_read_ += to_read;
    read += to_read;
    if (position_ == buffer_data_->wrap_position &&
        buffer_data_->total_written == data_size_) {
      // We've read all the beginning part, set the position to the middle part.
      // (The second condition above checks if the wrapping part is filled, i.e.
      // writing has wrapped.)
      position_ = buffer_data_->end_position;
    }
    if (read >= buffer_size) {
      DCHECK_EQ(read, buffer_size);
      return read;
    }
    if (read >= to_eow) {
      DCHECK_EQ(read, to_eow);
      DCHECK_EQ(total_read_, buffer_data_->total_written);
      return read;
    }
  }

  // Read from middle part.
  DCHECK_GE(position_, buffer_data_->wrap_position);
  if (position_ >= buffer_data_->end_position) {
    uint32 remaining_buffer_size = buffer_size - read;
    uint32 to_eof = data_size_ - position_;
    uint32 to_eow = buffer_data_->total_written - total_read_;
    uint32 to_read = Min3(remaining_buffer_size, to_eof, to_eow);
    memcpy(buffer_uint8 + read, buffer_data_->data + position_, to_read);
    position_ += to_read;
    total_read_ += to_read;
    read += to_read;
    if (position_ == data_size_) {
      // We've read all the middle part, set position to the end part.
      position_ = buffer_data_->wrap_position;
    }
    if (read >= buffer_size) {
      DCHECK_EQ(read, buffer_size);
      return read;
    }
    if (total_read_ >= buffer_data_->total_written) {
      DCHECK_EQ(total_read_, buffer_data_->total_written);
      return read;
    }
  }

  // Read from end part.
  DCHECK_GE(position_, buffer_data_->wrap_position);
  DCHECK_LT(position_, buffer_data_->end_position);
  uint32 remaining_buffer_size = buffer_size - read;
  uint32 to_eob = buffer_data_->end_position - position_;
  uint32 to_eow = buffer_data_->total_written - total_read_;
  uint32 to_read = Min3(remaining_buffer_size, to_eob, to_eow);
  memcpy(buffer_uint8 + read, buffer_data_->data + position_, to_read);
  position_ += to_read;
  total_read_ += to_read;
  read += to_read;
  DCHECK_LE(read, buffer_size);
  DCHECK_LE(total_read_, buffer_data_->total_written);
  return read;
}

void PartialCircularBuffer::Write(const void* buffer, uint32 buffer_size) {
  DCHECK(buffer_data_);
  const uint8* input = static_cast<const uint8*>(buffer);
  uint32 wrap_position = buffer_data_->wrap_position;
  uint32 cycle_size = data_size_ - wrap_position;

  // First write the non-wrapping part.
  if (position_ < wrap_position) {
    uint32 space_left = wrap_position - position_;
    uint32 write_size = std::min(buffer_size, space_left);
    DoWrite(input, write_size);
    input += write_size;
    buffer_size -= write_size;
  }

  // Skip the part that would overlap.
  if (buffer_size > cycle_size) {
    uint32 skip = buffer_size - cycle_size;
    input += skip;
    buffer_size -= skip;
    position_ = wrap_position + (position_ - wrap_position + skip) % cycle_size;
  }

  // Finally write the wrapping part.
  DoWrite(input, buffer_size);
}

void PartialCircularBuffer::DoWrite(const uint8* input, uint32 input_size) {
  DCHECK_LT(position_, data_size_);
  buffer_data_->total_written =
      std::min(buffer_data_->total_written + input_size, data_size_);

  // Write() skips any overlapping part, so this loop will run at most twice.
  while (input_size > 0) {
    uint32 space_left = data_size_ - position_;
    uint32 write_size = std::min(input_size, space_left);
    memcpy(buffer_data_->data + position_, input, write_size);
    input += write_size;
    input_size -= write_size;
    position_ += write_size;
    if (position_ >= data_size_) {
      DCHECK_EQ(position_, data_size_);
      position_ = buffer_data_->wrap_position;
    }
  }

  buffer_data_->end_position = position_;
}
