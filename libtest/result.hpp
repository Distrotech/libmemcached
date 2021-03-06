/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Data Differential YATL (i.e. libtest)  library
 *
 *  Copyright (C) 2012 Data Differential, http://datadifferential.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <libtest/fatal.hpp>
#include <libtest/result/base.hpp>
#include <libtest/result/fail.hpp>
#include <libtest/result/fatal.hpp>
#include <libtest/result/skip.hpp>
#include <libtest/result/success.hpp>

#define _SUCCESS throw libtest::__success(LIBYATL_DEFAULT_PARAM)

#define SKIP(...) \
do \
{ \
  throw libtest::__skipped(LIBYATL_DEFAULT_PARAM, __VA_ARGS__); \
} while (0)

#define FAIL(...) \
do \
{ \
  throw libtest::__failure(LIBYATL_DEFAULT_PARAM, __VA_ARGS__); \
} while (0)

#define FATAL(...) \
do \
{ \
  throw libtest::fatal(LIBYATL_DEFAULT_PARAM, __VA_ARGS__); \
} while (0)

#define FATAL_IF(__expression, ...) \
do \
{ \
  if ((__expression)) { \
    throw libtest::fatal(LIBYATL_DEFAULT_PARAM, (#__expression)); \
  } \
} while (0)

#define FATAL_IF_(__expression, ...) \
do \
{ \
  if ((__expression)) { \
    throw libtest::fatal(LIBYATL_DEFAULT_PARAM, __VA_ARGS__); \
  } \
} while (0)

#define fatal_assert(__assert) if((__assert)) {} else { throw libtest::fatal(LIBYATL_DEFAULT_PARAM, #__assert); }
