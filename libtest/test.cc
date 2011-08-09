/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  libtest
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <libtest/common.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
#include <fnmatch.h>
#include <iostream>

#include <signal.h>

#include <libtest/stats.h>
#include <libtest/signal.h>

#ifndef __INTEL_COMPILER
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

using namespace libtest;

static in_port_t global_port= 0;
static char global_socket[1024];

in_port_t default_port()
{
  return global_port;
}
 
void set_default_port(in_port_t port)
{
  global_port= port;
}

const char *default_socket()
{
  assert(global_socket[0]);
  return global_socket;
}

bool test_is_local()
{
  return (getenv("LIBTEST_LOCAL"));
}

void set_default_socket(const char *socket)
{
  if (socket)
  {
    strncpy(global_socket, socket, strlen(socket));
  }
}

static void stats_print(Stats *stats)
{
  if (stats->collection_failed == 0 and stats->collection_success == 0)
  {
    return;
  }

  Out << "\tTotal Collections\t\t\t\t" << stats->collection_total;
  Out << "\tFailed Collections\t\t\t\t" << stats->collection_failed;
  Out << "\tSkipped Collections\t\t\t\t" << stats->collection_skipped;
  Out << "\tSucceeded Collections\t\t\t\t" << stats->collection_success;
  Outn();
  Out << "Total\t\t\t\t" << stats->total;
  Out << "\tFailed\t\t\t" << stats->failed;
  Out << "\tSkipped\t\t\t" << stats->skipped;
  Out << "\tSucceeded\t\t" << stats->success;
}

static long int timedif(struct timeval a, struct timeval b)
{
  long us, s;

  us = (long)(a.tv_usec - b.tv_usec);
  us /= 1000;
  s = (long)(a.tv_sec - b.tv_sec);
  s *= 1000;
  return s + us;
}

const char *test_strerror(test_return_t code)
{
  switch (code) {
  case TEST_SUCCESS:
    return "ok";

  case TEST_FAILURE:
    return "failed";

  case TEST_MEMORY_ALLOCATION_FAILURE:
    return "memory allocation";

  case TEST_SKIPPED:
    return "skipped";

  case TEST_FATAL:
    break;
  }

  return "failed";
}

void create_core(void)
{
  if (getenv("LIBMEMCACHED_NO_COREDUMP") == NULL)
  {
    pid_t pid= fork();

    if (pid == 0)
    {
      abort();
    }
    else
    {
      while (waitpid(pid, NULL, 0) != pid) {};
    }
  }
}

static Framework *world= NULL;
int main(int argc, char *argv[])
{
  srandom((unsigned int)time(NULL));

  if (getenv("srcdir"))
  {
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s/%s", getenv("srcdir"), "tests");
    chdir(buffer);
  }
  else
  {
    chdir("tests");
  }

  world= new Framework();

  if (not world)
  {
    return EXIT_FAILURE;
  }

  libtest::SignalThread signal;
  if (not signal.setup())
  {
    return EXIT_FAILURE;
  }

  Stats stats;

  get_world(world);

  test_return_t error;
  void *creators_ptr= world->create(error);

  switch (error)
  {
  case TEST_SUCCESS:
    break;

  case TEST_SKIPPED:
    Out << "SKIP " << argv[0];
    delete world;
    return EXIT_SUCCESS;

  case TEST_FATAL:
  case TEST_FAILURE:
  case TEST_MEMORY_ALLOCATION_FAILURE:
    Error << argv[0] << "create() failed";
    delete world;
    return EXIT_FAILURE;
  }

  char *collection_to_run= NULL;
  if (argc > 1)
  {
    collection_to_run= argv[1];
  }
  else if (getenv("TEST_COLLECTION"))
  {
    collection_to_run= getenv("TEST_COLLECTION");
  }

  if (collection_to_run)
  {
    Out << "Only testing " <<  collection_to_run;
  }

  char *wildcard= NULL;
  if (argc == 3)
  {
    wildcard= argv[2];
  }

  for (collection_st *next= world->collections; next->name and (not signal.is_shutdown()); next++)
  {
    test_return_t collection_rc= TEST_SUCCESS;
    bool failed= false;
    bool skipped= false;

    if (collection_to_run && fnmatch(collection_to_run, next->name, 0))
      continue;

    stats.collection_total++;

    collection_rc= world->startup(creators_ptr);

    if (collection_rc == TEST_SUCCESS and next->pre)
    {
      collection_rc= world->runner()->pre(next->pre, creators_ptr);
    }

    switch (collection_rc)
    {
    case TEST_SUCCESS:
      break;

    case TEST_FATAL:
    case TEST_FAILURE:
      Out << next->name << " [ failed ]";
      failed= true;
      signal.set_shutdown(SHUTDOWN_GRACEFUL);
      goto cleanup;

    case TEST_SKIPPED:
      Out << next->name << " [ skipping ]";
      skipped= true;
      goto cleanup;

    case TEST_MEMORY_ALLOCATION_FAILURE:
      test_assert(0, "Allocation failure, or unknown return");
    }

    Out << "Collection: " << next->name;

    for (test_st *run= next->tests; run->name; run++)
    {
      struct timeval start_time, end_time;
      long int load_time= 0;

      if (wildcard && fnmatch(wildcard, run->name, 0))
      {
        continue;
      }

      test_return_t return_code;
      if (test_success(return_code= world->item.startup(creators_ptr)))
      {
        if (test_success(return_code= world->item.flush(creators_ptr, run)))
        {
          // @note pre will fail is SKIPPED is returned
          if (test_success(return_code= world->item.pre(creators_ptr)))
          {
            { // Runner Code
              gettimeofday(&start_time, NULL);
              assert(world->runner());
              assert(run->test_fn);
              return_code= world->runner()->run(run->test_fn, creators_ptr);
              gettimeofday(&end_time, NULL);
              load_time= timedif(end_time, start_time);
            }
          }

          // @todo do something if post fails
          (void)world->item.post(creators_ptr);
        }
        else if (return_code == TEST_SKIPPED)
        { }
        else if (return_code == TEST_FAILURE)
        {
          Error << " item.flush(failure)";
          signal.set_shutdown(SHUTDOWN_GRACEFUL);
        }
      }
      else if (return_code == TEST_SKIPPED)
      { }
      else if (return_code == TEST_FAILURE)
      {
        Error << " item.startup(failure)";
        signal.set_shutdown(SHUTDOWN_GRACEFUL);
      }

      stats.total++;

      switch (return_code)
      {
      case TEST_SUCCESS:
        Out << "\tTesting " << run->name <<  "\t\t\t\t\t" << load_time / 1000 << "." << load_time % 1000 << "[ " << test_strerror(return_code) << " ]";
        stats.success++;
        break;

      case TEST_FATAL:
      case TEST_FAILURE:
        stats.failed++;
        failed= true;
        Out << "\tTesting " << run->name <<  "\t\t\t\t\t" << "[ " << test_strerror(return_code) << " ]";
        break;

      case TEST_SKIPPED:
        stats.skipped++;
        skipped= true;
        Out << "\tTesting " << run->name <<  "\t\t\t\t\t" << "[ " << test_strerror(return_code) << " ]";
        break;

      case TEST_MEMORY_ALLOCATION_FAILURE:
        test_assert(0, "Memory Allocation Error");
      }

      if (test_failed(world->on_error(return_code, creators_ptr)))
      {
        Error << "Failed while running on_error()";
        signal.set_shutdown(SHUTDOWN_GRACEFUL);
        break;
      }
    }

    (void) world->runner()->post(next->post, creators_ptr);

cleanup:
    if (failed == false and skipped == false)
    {
      stats.collection_success++;
    }

    if (failed)
    {
      stats.collection_failed++;
    }

    if (skipped)
    {
      stats.collection_skipped++;
    }

    world->shutdown(creators_ptr);
    Outn();
  }

  if (not signal.is_shutdown())
  {
    signal.set_shutdown(SHUTDOWN_GRACEFUL);
  }

  int exit_code= EXIT_SUCCESS;
  shutdown_t status= signal.get_shutdown();
  if (status == SHUTDOWN_FORCED)
  {
    Out << "Tests were aborted.";
    exit_code= EXIT_FAILURE;
  }
  else if (stats.collection_failed)
  {
    Out << "Some test failed.";
    exit_code= EXIT_FAILURE;
  }
  else if (stats.collection_skipped and stats.collection_failed and stats.collection_success)
  {
    Out << "Some tests were skipped.";
  }
  else if (stats.collection_success and stats.collection_failed == 0)
  {
    Out << "All tests completed successfully.";
  }

  stats_print(&stats);

  delete world;

  Outn(); // Generate a blank to break up the messages if make check/test has been run

  return exit_code;
}
