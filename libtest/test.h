/* 
 * uTest Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 */

#pragma once

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <libtest/stream.h>

LIBTEST_API
const char* default_socket();

LIBTEST_API
void set_default_socket(const char *socket);


/**
  A structure describing the test case.
*/
struct test_st {
  const char *name;
  bool requires_flush;
  test_callback_fn *test_fn;
};

LIBTEST_API
bool test_is_local(void);

#define test_assert_errno(A) \
do \
{ \
  if ((A)) { \
    fprintf(stderr, "\n%s:%d: Assertion failed for %s: ", __FILE__, __LINE__, __func__);\
    perror(#A); \
    fprintf(stderr, "\n"); \
    create_core(); \
    assert((A)); \
  } \
} while (0)

#define test_assert(A, B) \
do \
{ \
  if ((A)) { \
    fprintf(stderr, "\n%s:%d: Assertion failed %s, with message %s, in %s", __FILE__, __LINE__, (B), #A, __func__ );\
    fprintf(stderr, "\n"); \
    create_core(); \
    assert((A)); \
  } \
} while (0)

#define test_truth(A) \
do \
{ \
  if (! (A)) { \
    fprintf(stderr, "\n%s:%d: Assertion \"%s\" failed, in %s\n", __FILE__, __LINE__, #A, __func__);\
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_true(A) \
do \
{ \
  if (! (A)) { \
    fprintf(stderr, "\n%s:%d: Assertion \"%s\" failed, in %s\n", __FILE__, __LINE__, #A, __func__);\
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

template <class T_comparable, class T_hint>
bool _true_hint(const char *file, int line, const char *func, T_comparable __expected, const char *assertation_label,  T_hint __hint)
{
  if (__expected == false)
  {
    libtest::stream::make_cerr(file, line, func) << "Assertation  \"" << assertation_label << "\" failed, hint: " << __hint;
    return false;
  }

  return true;
}

#define test_true_got(__expected, __hint) \
do \
{ \
  if (not _true_hint(__FILE__, __LINE__, __func__, ((__expected)), #__expected, ((__hint)))) \
  { \
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_skip(A,B) \
do \
{ \
  if ((A) != (B)) \
  { \
    return TEST_SKIPPED; \
  } \
} while (0)

#define test_fail(A) \
do \
{ \
  if (1) { \
    fprintf(stderr, "\n%s:%d: Failed with %s, in %s\n", __FILE__, __LINE__, #A, __func__);\
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)


#define test_false(A) \
do \
{ \
  if ((A)) { \
    fprintf(stderr, "\n%s:%d: Assertion failed %s, in %s\n", __FILE__, __LINE__, #A, __func__);\
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_false_with(A,B) \
do \
{ \
  if ((A)) { \
    fprintf(stderr, "\n%s:%d: Assertion failed %s with %s\n", __FILE__, __LINE__, #A, (B));\
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

template <class T_comparable>
bool _compare(const char *file, int line, const char *func, T_comparable __expected, T_comparable __actual)
{
  if (__expected != __actual)
  {
    libtest::stream::make_cerr(file, line, func) << "Expected \"" << __expected << "\" got \"" << __actual << "\"";
    return false;
  }

  return true;
}

template <class T_comparable>
bool _compare_zero(const char *file, int line, const char *func, T_comparable __actual)
{
  if (T_comparable(0) != __actual)
  {
    libtest::stream::make_cerr(file, line, func) << "Expected 0 got \"" << __actual << "\"";
    return false;
  }

  return true;
}

template <class T_comparable, class T_hint>
bool _compare_hint(const char *file, int line, const char *func, T_comparable __expected, T_comparable __actual, T_hint __hint)
{
  if (__expected != __actual)
  {
    libtest::stream::make_cerr(file, line, func) << "Expected \"" << __expected << "\" got \"" << __actual << "\" Additionally: \"" << __hint << "\"";

    return false;
  }

  return true;
}

#define test_compare(__expected, __actual) \
do \
{ \
  if (not _compare(__FILE__, __LINE__, __func__, ((__expected)), ((__actual)))) \
  { \
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_zero(__actual) \
do \
{ \
  if (not _compare_zero(__FILE__, __LINE__, __func__, ((__actual)))) \
  { \
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_compare_got(__expected, __actual, __hint) \
do \
{ \
  if (not _compare_hint(__FILE__, __LINE__, __func__, (__expected), (__actual), (__hint))) \
  { \
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)


#define test_strcmp(A,B) \
do \
{ \
  if (strcmp((A), (B))) \
  { \
    fprintf(stderr, "\n%s:%d: Expected %s, got %s\n", __FILE__, __LINE__, (A), (B)); \
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_memcmp(A,B,C) \
do \
{ \
  if (memcmp((A), (B), (C))) \
  { \
    fprintf(stderr, "\n%s:%d: %.*s -> %.*s\n", __FILE__, __LINE__, (int)(C), (char *)(A), (int)(C), (char *)(B)); \
    create_core(); \
    return TEST_FAILURE; \
  } \
} while (0)

#define test_return_if(__test_return_t) \
do \
{ \
  if ((__test_return_t) != TEST_SUCCESS) \
  { \
    return __test_return_t; \
  } \
} while (0)

