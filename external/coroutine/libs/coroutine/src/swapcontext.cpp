//  Copyright (c) 2007-2012 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#if defined(__GNUC__)

#if defined(__i386__)
#include "swapcontext32.cpp"
#elif defined(__x86_64__)
#include "swapcontext64.cpp"
#else
#error Undefined platform (should be __386__ or __x86_64__)
#endif

#endif
