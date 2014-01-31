#
# Copyright (C) 2013 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

{
  'variables': {
    'heap_files': [
      'AddressSanitizer.h',
      'Handle.h',
      'Heap.cpp',
      'Heap.h',
      'HeapExport.h',
      'ThreadState.cpp',
      'ThreadState.h',
      'Visitor.cpp',
      'Visitor.h',
    ],
    'heap_test_files': [
      'HeapTest.cpp',
    ],
    'conditions': [
      ['target_arch == "arm"', {
        'heap_asm_files': [
          'asm/SaveRegisters_arm.S',
        ],
      }],
      ['target_arch == "a64"', {
        'heap_asm_files': [
          'asm/SaveRegisters_a64.S',
        ],
      }],
      ['target_arch == "mipsel"', {
        'heap_asm_files': [
          'asm/SaveRegisters_mips.S',
        ],
      }],
      ['target_arch == "ia32" or target_arch == "x64"', {
        'heap_asm_files': [
          'asm/SaveRegisters_x86.asm',
        ],
      }],
    ],
  },
}
