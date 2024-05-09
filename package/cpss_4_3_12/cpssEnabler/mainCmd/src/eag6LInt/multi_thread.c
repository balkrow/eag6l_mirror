#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/select.h>


#include "multi_thread.h"

/* Thread types. */
#define MULTI_THREAD_READ  0
#define MULTI_THREAD_WRITE 1
#define MULTI_THREAD_TIMER 2
#define MULTI_THREAD_EVENT 3
#define MULTI_THREAD_READY 4
#define MULTI_THREAD_UNUSED 5

/* Make multi_thread master. */
struct multi_thread_master * multi_thread_make_master() 
{
	struct multi_thread_master *new;
	new = (struct multi_thread_master *)malloc( sizeof(struct multi_thread_master));
	bzero (new, sizeof (struct multi_thread_master));
	return new;
}

static void multi_thread_list_add (struct multi_thread_list *list, struct multi_thread *multi_thread)
{
  multi_thread->next = NULL;
  multi_thread->prev = list->tail;
  if (list->tail)
    list->tail->next = multi_thread;
  else
    list->head = multi_thread;
  list->tail = multi_thread;
  list->count++;
}


void multi_thread_list_add_before (struct multi_thread_list *list, 
			struct multi_thread *point, 
			struct multi_thread *multi_thread)
{
  multi_thread->next = point;
  multi_thread->prev = point->prev;
  if (point->prev)
    point->prev->next = multi_thread;
  else
    list->head = multi_thread;
  point->prev = multi_thread;
  list->count++;
}


struct multi_thread * multi_thread_list_delete (struct multi_thread_list *list, struct multi_thread *multi_thread)
{
  if (multi_thread->next)
    multi_thread->next->prev = multi_thread->prev;
  else
    list->tail = multi_thread->prev;
  if (multi_thread->prev)
    multi_thread->prev->next = multi_thread->next;
  else
    list->head = multi_thread->next;
  multi_thread->next = multi_thread->prev = NULL;
  list->count--;
  return multi_thread;
}


static void multi_thread_clean_unuse (struct multi_thread_master *master)
{
  struct multi_thread *multi_thread;

  multi_thread = master->unuse.head;
  while (multi_thread)
    {
      struct multi_thread *unuset;

      unuset = multi_thread;
      multi_thread = unuset->next;

      multi_thread_list_delete (&master->unuse, unuset);
     free((void *)unuset);
      master->alloc--;
    }
}


static void multi_thread_add_unuse (struct multi_thread_master *master, struct multi_thread *multi_thread)
{
  assert (master != NULL);
  assert (multi_thread->next == NULL);
  assert (multi_thread->prev == NULL);
  assert (multi_thread->type == MULTI_THREAD_UNUSED);
  multi_thread_list_add (&master->unuse, multi_thread);
}


void multi_thread_destroy_master (struct multi_thread_master *master)
{
  struct multi_thread *multi_thread;

  multi_thread = master->read.head;
  while (multi_thread)
    {
      struct multi_thread *readt;

      readt = multi_thread;
      multi_thread = readt->next;

      multi_thread_list_delete (&master->read, readt);
      readt->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, readt);
    }

  multi_thread = master->write.head;
  while (multi_thread)
    {
      struct multi_thread *writet;

      writet = multi_thread;
      multi_thread = writet->next;

      multi_thread_list_delete (&master->write, writet);
      writet->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, writet);
    }

  multi_thread = master->timer.head;
  while (multi_thread)
    {
      struct multi_thread *timert;

      timert = multi_thread;
      multi_thread = timert->next;

      multi_thread_list_delete (&master->timer, timert);
      timert->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, timert);
    }

  multi_thread = master->event.head;
  while (multi_thread)
    {
      struct multi_thread *eventt;

      eventt = multi_thread;
      multi_thread = eventt->next;

      multi_thread_list_delete (&master->event, eventt);
      eventt->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, eventt);
    }

  multi_thread = master->ready.head;
  while (multi_thread)
    {
      struct multi_thread *readyt;

      readyt = multi_thread;
      multi_thread = readyt->next;

      multi_thread_list_delete (&master->ready, readyt);
      readyt->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, readyt);
    }

  multi_thread_clean_unuse (master);
  free((void *)master);
}

/* Delete top of the list and return it. */
struct multi_thread *
multi_thread_trim_head (struct multi_thread_list *list)
{
  if (list->head)
    return multi_thread_list_delete (list, list->head);
  return NULL;
}

/* Make new multi_thread. */
struct multi_thread *
multi_thread_new (struct multi_thread_master *master)
{
  struct multi_thread *new;

  if (master->unuse.head)
    return (multi_thread_trim_head (&master->unuse));
  new = (struct multi_thread *)malloc ( sizeof (struct multi_thread));
  bzero (new, sizeof (struct multi_thread));
  master->alloc++;
  return new;
}

/* Add new read multi_thread. */
struct multi_thread *
multi_thread_add_read (struct multi_thread_master *master, 
		 int (*func)(struct multi_thread *),
		 void *arg,
		 int fd)
{
  struct multi_thread *multi_thread;

  assert (master != NULL);

  if (FD_ISSET (fd, &master->readfd))
    {
      return NULL;
    }

  multi_thread = multi_thread_new (master);
  multi_thread->type = MULTI_THREAD_READ;
  multi_thread->id = 0;
  multi_thread->master = master;
  multi_thread->func = func;
  multi_thread->arg = arg;
  FD_SET (fd, &master->readfd);
  multi_thread->u.fd = fd;
  multi_thread_list_add (&master->read, multi_thread);

  return multi_thread;
}

/* Add new write multi_thread. */
struct multi_thread *
multi_thread_add_write (struct multi_thread_master *master,
		 int (*func)(struct multi_thread *),
		 void *arg,
		 int fd)
{
  struct multi_thread *multi_thread;

  assert (master != NULL);

  if (FD_ISSET (fd, &master->writefd))
    {
      return NULL;
    }

  multi_thread = multi_thread_new (master);
  multi_thread->type = MULTI_THREAD_WRITE;
  multi_thread->id = 0;
  multi_thread->master = master;
  multi_thread->func = func;
  multi_thread->arg = arg;
  FD_SET (fd, &master->writefd);
  multi_thread->u.fd = fd;
  multi_thread_list_add (&master->write, multi_thread);

  return multi_thread;
}

/* timer compare */
static int
multi_thread_timer_cmp (struct timespec a, struct timespec b)
{
  if (a.tv_sec > b.tv_sec) 
    return 1;
  if (a.tv_sec < b.tv_sec)
    return -1;
  if (a.tv_nsec > b.tv_nsec)
    return 1;
  if (a.tv_nsec < b.tv_nsec)
    return -1;
  return 0;
}

/* Add timer event multi_thread. */
struct multi_thread *
multi_thread_add_timer (struct multi_thread_master *master,
		  int (*func)(struct multi_thread *),
		  void *arg,
		  long timer)
{

 struct timespec timer_now_spec;

  struct multi_thread *multi_thread;
  struct multi_thread *multi_thread_time;

  assert (master != NULL);

  multi_thread = multi_thread_new (master);
  multi_thread->type = MULTI_THREAD_TIMER;
  multi_thread->id = 0;
  multi_thread->master = master;
  multi_thread->func = func;
  multi_thread->arg = arg;

  /* Do we need jitter here? */

  clock_gettime(CLOCK_MONOTONIC , &timer_now_spec);
  timer_now_spec.tv_sec += timer;
 
  multi_thread->u.sands = timer_now_spec;

  for (multi_thread_time = master->timer.head; multi_thread_time; multi_thread_time = multi_thread_time->next)
    if (multi_thread_timer_cmp (multi_thread->u.sands, multi_thread_time->u.sands) <= 0)
      break;

  if (multi_thread_time)
    multi_thread_list_add_before (&master->timer, multi_thread_time, multi_thread);
  else
    multi_thread_list_add (&master->timer, multi_thread);

  return multi_thread;
}


/* Add timer event multi_thread. */
struct multi_thread *
multi_thread_add_timer_msec (struct multi_thread_master *master,
		  int (*func)(struct multi_thread *),
		  void *arg,
		  long timer)
{
  
  struct timespec timer_now_spec;

  struct multi_thread *multi_thread;
  struct multi_thread *multi_thread_time;
  assert (master != NULL);

  multi_thread = multi_thread_new (master);
  multi_thread->type = MULTI_THREAD_TIMER;
  multi_thread->id = 0;
  multi_thread->master = master;
  multi_thread->func = func;
  multi_thread->arg = arg;

  /* Do we need jitter here? */

	clock_gettime(CLOCK_MONOTONIC , &timer_now_spec);
	timer_now_spec.tv_sec += timer/1000;
	timer_now_spec.tv_nsec += 1000000*(timer % 1000000);
    multi_thread->u.sands = timer_now_spec;


  for (multi_thread_time = master->timer.head; multi_thread_time; multi_thread_time = multi_thread_time->next)
    if (multi_thread_timer_cmp (multi_thread->u.sands, multi_thread_time->u.sands) <= 0)
      break;

  if (multi_thread_time)
    multi_thread_list_add_before (&master->timer, multi_thread_time, multi_thread);
  else
    multi_thread_list_add (&master->timer, multi_thread);

  return multi_thread;
}


/* Add simple event multi_thread. */
struct multi_thread *
multi_thread_add_event (struct multi_thread_master *master,
		  int (*func)(struct multi_thread *), 
		  void *arg,
		  int val)
{
  struct multi_thread *multi_thread;

  assert (master != NULL);

  multi_thread = multi_thread_new (master);
  multi_thread->type = MULTI_THREAD_EVENT;
  multi_thread->id = 0;
  multi_thread->master = master;
  multi_thread->func = func;
  multi_thread->arg = arg;
  multi_thread->u.val = val;
  multi_thread_list_add (&master->event, multi_thread);

  return multi_thread;
}

void
multi_thread_cancel (struct multi_thread *multi_thread)
{
  /**/
  switch (multi_thread->type)
    {
    case MULTI_THREAD_READ:
      assert (FD_ISSET (multi_thread->u.fd, &multi_thread->master->readfd));
      FD_CLR (multi_thread->u.fd, &multi_thread->master->readfd);
      multi_thread_list_delete (&multi_thread->master->read, multi_thread);
      break;
    case MULTI_THREAD_WRITE:
      assert (FD_ISSET (multi_thread->u.fd, &multi_thread->master->writefd));
      FD_CLR (multi_thread->u.fd, &multi_thread->master->writefd);
      multi_thread_list_delete (&multi_thread->master->write, multi_thread);
      break;
    case MULTI_THREAD_TIMER:
      multi_thread_list_delete (&multi_thread->master->timer, multi_thread);
      break;
    case MULTI_THREAD_EVENT:
      multi_thread_list_delete (&multi_thread->master->event, multi_thread);
      break;
    case MULTI_THREAD_READY:
      multi_thread_list_delete (&multi_thread->master->ready, multi_thread);
      break;
    default:
      break;
    }
  multi_thread->type = MULTI_THREAD_UNUSED;
  multi_thread_add_unuse (multi_thread->master, multi_thread);


}

void
multi_thread_cancel_event (struct multi_thread_master *master, void *arg)
{
  struct multi_thread *multi_thread;

  multi_thread = master->event.head;
  while (multi_thread)
    {
      struct multi_thread *eventt;

      eventt = multi_thread;
      multi_thread = eventt->next;

      if (eventt->arg == arg)
	{
	  multi_thread_list_delete (&master->event, eventt);
	  eventt->type = MULTI_THREAD_UNUSED;
	  multi_thread_add_unuse (master, eventt);
	}
    }
}


#define TIMER_SEC_NANO 1000000000

struct timespec
multi_thread_timer_sub (struct timespec a, struct timespec b)
{
  struct timespec ret;

  ret.tv_nsec = a.tv_nsec - b.tv_nsec;
  ret.tv_sec = a.tv_sec - b.tv_sec;

  if (ret.tv_nsec < 0) {
    ret.tv_nsec += TIMER_SEC_NANO;
    ret.tv_sec--;
  }

  return ret;
}


struct multi_thread *
multi_thread_fetch (struct multi_thread_master *master, struct multi_thread *fetch)
{
  int ret;
  struct multi_thread *multi_thread;
  fd_set readfd;
  fd_set writefd;
  fd_set exceptfd;
  struct timespec timer_now;
  struct timespec timer_min;
  struct timespec *timer_wait;


  assert (master != NULL);

 retry: 
  /* If there is event process it first. */
  while ((multi_thread = multi_thread_trim_head (&master->event)))
    {
      *fetch = *multi_thread;
      multi_thread->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, multi_thread);
      return fetch;
    }

  /* If there is ready multi_threads process them */
  while ((multi_thread = multi_thread_trim_head (&master->ready)))
    {
      *fetch = *multi_thread;
      multi_thread->type = MULTI_THREAD_UNUSED;
      multi_thread_add_unuse (master, multi_thread);
      return fetch;
    }



  if (master->timer.head)
    {
	clock_gettime(CLOCK_MONOTONIC , &timer_now);

      timer_min = master->timer.head->u.sands;
      timer_min = multi_thread_timer_sub (timer_min, timer_now);
      if (timer_min.tv_sec < 0)
	{
	  timer_min.tv_sec = 0;
	  timer_min.tv_nsec = 10;
	}
      timer_wait = &timer_min;
    }
  else
    {
      timer_wait = NULL;
    }

  /* Call select function. */
  readfd = master->readfd;
  writefd = master->writefd;
  exceptfd = master->exceptfd;


  ret = pselect (FD_SETSIZE, &readfd, &writefd, &exceptfd, timer_wait,NULL);

  if (ret < 0)
    {
      if (errno != EINTR)
	{
	  /* Real error. */
	  assert (0);
	}
      /* Signal is coming. */
      goto retry;
    }


  multi_thread = master->read.head;
  while (multi_thread)
    {
      struct multi_thread *readt;
      
      readt = multi_thread;
      multi_thread = readt->next;

      if (FD_ISSET (readt->u.fd, &readfd))
	{
	  assert (FD_ISSET (readt->u.fd, &master->readfd));
	  FD_CLR(readt->u.fd, &master->readfd);
	  multi_thread_list_delete (&master->read, readt);
	  multi_thread_list_add (&master->ready, readt);
	  readt->type = MULTI_THREAD_READY;
	}
    }
 

  multi_thread = master->write.head;
  while (multi_thread)
    {
      struct multi_thread *writet;

      writet = multi_thread;
      multi_thread = writet->next;

      if (FD_ISSET (writet->u.fd, &writefd))
	{
	  assert (FD_ISSET (writet->u.fd, &master->writefd));
	  FD_CLR(writet->u.fd, &master->writefd);
	  multi_thread_list_delete (&master->write, writet);
	  multi_thread_list_add (&master->ready, writet);
	  writet->type = MULTI_THREAD_READY;
	}
    }


  clock_gettime(CLOCK_MONOTONIC , &timer_now);
  multi_thread = master->timer.head;
  while (multi_thread)
    {
      struct multi_thread *timert;

      timert = multi_thread;
      multi_thread = timert->next;

      if (multi_thread_timer_cmp (timer_now, timert->u.sands) >= 0)
	{
	  multi_thread_list_delete (&master->timer, timert);
	  multi_thread_list_add (&master->ready, timert);
	  timert->type = MULTI_THREAD_READY;
	}
    }

  multi_thread = multi_thread_trim_head (&master->ready);

   if (!multi_thread)
    goto retry;

  *fetch = *multi_thread;
  multi_thread->type = MULTI_THREAD_UNUSED;
  multi_thread_add_unuse (master, multi_thread);

  
  return fetch;
}



unsigned long int
multi_thread_get_id (void)
{
  static unsigned long int counter = 0;
  return ++counter;
}

int multi_thread_call (struct multi_thread *multi_thread)
{
  multi_thread->id = multi_thread_get_id ();
  return (*multi_thread->func) (multi_thread);
}

struct multi_thread *
multi_thread_execute (int (*func)(struct multi_thread *), 
                void *arg,
                int val)
{
  struct multi_thread dummy; 

  memset (&dummy, 0, sizeof (struct multi_thread));

  dummy.type = MULTI_THREAD_EVENT;
  dummy.id = 0;
  dummy.master = (struct multi_thread_master *)NULL;
  dummy.func = func;
  dummy.arg = arg;
  dummy.u.val = val;
  multi_thread_call (&dummy);     /* execute immediately */

  return (struct multi_thread *)NULL;
}
