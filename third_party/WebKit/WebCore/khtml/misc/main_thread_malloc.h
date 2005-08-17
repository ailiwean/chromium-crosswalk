// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 2005 Apple Computer, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#ifndef KHTML_MAIN_THREAD_MALLOC_H
#define KHTML_MAIN_THREAD_MALLOC_H

// This is a copy of dlmalloc, a fast single-threaded malloc implementation.
// Therefore, these functions should only be used on the main thread.

#include <stdlib.h>

namespace khtml {

void *main_thread_malloc(size_t n);
void *main_thread_calloc(size_t n_elements, size_t element_size);
void main_thread_free(void* p);
void *main_thread_realloc(void* p, size_t n);

#define MAIN_THREAD_ALLOCATED \
void* operator new(size_t s) { return khtml::main_thread_malloc(s); } \
void operator delete(void* p) { khtml::main_thread_free(p); }

}

#endif /* KHTML_MAIN_THREAD_MALLOC_H */
