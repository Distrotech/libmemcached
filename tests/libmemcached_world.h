/* libMemcached Functions Test
 * Copyright (C) 2006-2009 Brian Aker
 * All rights reserved.
 *
 * Use and distribution licensed under the BSD license.  See
 * the COPYING file in the parent directory for full text.
 * 
 * Description: This is the startup bits for any libmemcached test.
 *
 */

#ifdef	__cplusplus
extern "C" {
#endif

/* The structure we use for the test system */
typedef struct
{
  server_startup_st construct;
  memcached_st *memc;
} libmemcached_test_container_st;

/* Prototypes for functions we will pass to test framework */
libmemcached_test_container_st *world_create(void);
test_return_t world_collection_startup(libmemcached_test_container_st *);
test_return_t world_flush(libmemcached_test_container_st *container);
test_return_t world_pre_run(libmemcached_test_container_st *);

test_return_t world_post_run(libmemcached_test_container_st *);
test_return_t world_on_error(test_return_t, libmemcached_test_container_st *);
test_return_t world_destroy(libmemcached_test_container_st *);

static libmemcached_test_container_st global_container;

libmemcached_test_container_st *world_create(void)
{
  memset(&global_container, 0, sizeof(global_container));
  global_container.construct.count= SERVERS_TO_CREATE;
  global_container.construct.udp= 0;
  server_startup(&global_container.construct);

  assert(global_container.construct.servers);

  return &global_container;
}


test_return_t world_collection_startup(libmemcached_test_container_st *container)
{
  memcached_return_t rc;
  container->memc= memcached_create(NULL);
  test_truth((container->memc != NULL));

  rc= memcached_server_push(container->memc, container->construct.servers);
  test_truth(rc == MEMCACHED_SUCCESS);

  return TEST_SUCCESS;
}

test_return_t world_flush(libmemcached_test_container_st *container)
{
  memcached_flush(container->memc, 0);
  memcached_quit(container->memc);

  return TEST_SUCCESS;
}

test_return_t world_pre_run(libmemcached_test_container_st *container)
{
  uint32_t loop;

  for (loop= 0; loop < memcached_server_list_count(container->construct.servers); loop++)
  {
    test_truth(container->memc->hosts[loop].fd == -1);
    test_truth(container->memc->hosts[loop].cursor_active == 0);
  }

  return TEST_SUCCESS;
}


test_return_t world_post_run(libmemcached_test_container_st *container)
{
  assert(container->memc);

  return TEST_SUCCESS;
}

test_return_t world_on_error(test_return_t test_state, libmemcached_test_container_st *container)
{
  (void)test_state;
  memcached_free(container->memc);
  
  return TEST_SUCCESS;
}

test_return_t world_destroy(libmemcached_test_container_st *container)
{
  server_startup_st *construct= &container->construct;
  memcached_server_st *servers= container->construct.servers;
  memcached_server_list_free(servers);

  server_shutdown(construct);

  return TEST_SUCCESS;
}

typedef test_return_t (*libmemcached_test_callback_fn)(memcached_st *);
static test_return_t _runner_default(libmemcached_test_callback_fn func, libmemcached_test_container_st *container)
{
  if (func)
  {
    return func(container->memc);
  }
  else
  {
    return TEST_SUCCESS;
  }
}

#ifdef	__cplusplus
}
#endif

#ifdef	__cplusplus

static world_runner_st defualt_libmemcached_runner= {
  reinterpret_cast<test_callback_runner_fn>(_runner_default),
  reinterpret_cast<test_callback_runner_fn>(_runner_default),
  reinterpret_cast<test_callback_runner_fn>(_runner_default)
};

#else

static world_runner_st defualt_libmemcached_runner= {
  (test_callback_runner_fn)_runner_default,
  (test_callback_runner_fn)_runner_default,
  (test_callback_runner_fn)_runner_default
};

#endif