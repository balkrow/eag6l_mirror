
/* Linked list of multi_thread. */
struct multi_thread_list
{
  struct multi_thread *head;
  struct multi_thread *tail;
  int count;
};

/* Master of the multi theads. */
struct multi_thread_master
{
  struct multi_thread_list read;
  struct multi_thread_list write;
  struct multi_thread_list timer;
  struct multi_thread_list event;
  struct multi_thread_list ready;
  struct multi_thread_list unuse;
  fd_set readfd;
  fd_set writefd;
  fd_set exceptfd;
  unsigned long alloc;
};

/* Thread itself. */
struct multi_thread
{
  unsigned long id;
  unsigned char type;		/* multi_thread type */
  struct multi_thread *next;		/* next pointer of the multi_thread */
  struct multi_thread *prev;		/* previous pointer of the multi_thread */
  struct multi_thread_master *master;	/* pointer to the struct multi_thread_master. */
  int (*func) (struct multi_thread *); /* event function */
  void *arg;			/* event argument */
  union {
    int val;			/* second argument of the event. */
    int fd;			/* file descriptor in case of read/write. */
    struct timespec sands;	/* rest of time sands value. */
  } u;
};

/* Thread types. */
#define MULTI_THREAD_READ           0
#define MULTI_THREAD_WRITE          1
#define MULTI_THREAD_TIMER          2
#define MULTI_THREAD_EVENT          3
#define MULTI_THREAD_READY          4
#define MULTI_THREAD_UNUSED         5

/* Thread yield time.  */
#define MULTI_THREAD_YIELD_TIME_SLOT     100 * 1000L /* 100ms */

/* Macros. */
#define MULTI_THREAD_ARG(X) ((X)->arg)
#define MULTI_THREAD_FD(X)  ((X)->u.fd)
#define MULTI_THREAD_VAL(X) ((X)->u.val)

/* Prototypes. */
struct multi_thread_master *multi_thread_make_master (void);

struct multi_thread *
multi_thread_add_read (struct multi_thread_master *m, 
		 int (*func)(struct multi_thread *),
		 void *arg,
		 int fd);

struct multi_thread *
multi_thread_add_write (struct multi_thread_master *m,
		 int (*func)(struct multi_thread *),
		 void *arg,
		 int fd);

struct multi_thread *
multi_thread_add_timer (struct multi_thread_master *m,
		  int (*func)(struct multi_thread *),
		  void *arg,
		  long timer);


struct multi_thread *
multi_thread_add_timer_msec (struct multi_thread_master *m,
          int (*func)(struct multi_thread *),
          void *arg,
          long timer);


struct multi_thread *
multi_thread_add_event (struct multi_thread_master *m,
		  int (*func)(struct multi_thread *), 
		  void *arg,
		  int val);

void
multi_thread_cancel (struct multi_thread *multi_thread);

void
multi_thread_cancel_event (struct multi_thread_master *m, void *arg);

struct multi_thread *
multi_thread_fetch (struct multi_thread_master *m, 
	      struct multi_thread *fetch);

int
multi_thread_call (struct multi_thread *multi_thread);

struct multi_thread *
multi_thread_execute (int (*func)(struct multi_thread *), 
		  void *arg,
		  int val);

