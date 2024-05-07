/**

   Macro for defining non-keyed collections of objects.

   File: macsec_coll.h

   @description
   A macro for defining a collection type and associated
   initialization, iteration, insert and remove functions for a
   specific data type.

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _MACSEC_COLL_H
#define _MACSEC_COLL_H
#include "macsec_clib.h"

typedef struct MacsecCollNode {
  struct MacsecCollNode *prev;
  struct MacsecCollNode *next;
} MacsecCollNode;

/** Define type and functions for a non-keyed collection.

    @param colltype
    Typedef name of the collection type to be defined.

    @param funcprefix
    Prefix that is prepended to the names of the functions to be defined.

    @maxelems
    The maximum number of elements the collection can hold.

    @datatype
    The type of an element to be stored in the collection.

    @description
    This macro defines a collection type that contains space for
    storing maxelements elements of type datatype and associated
    collection management overhead. It also defines the following
    functions:

    static inline void
    funcprefix_init(colltype *coll)

      Initialize a collection. This function must be called before
      using the other functions.

    static inline datatype *
    funcprefix_insert(colltype *coll)

      Insert a new element to the collection and return pointer to
      it. If no more elements can be inserted, return NULL.

    static inline void
    funcprefix_remove(colltype *coll, datatype *data)

      Remove an element from the collection. The parameter data must
      be a valid pointer to an element previously returned by
      funcprefix_insert(), funcprefix_next() or passed by
      funcprefix_foreach() to its parameter function func. Note: after
      removing an element returned by funcprefix_next() it must not be
      used as the parameter prev to a subsequent call to
      funcprefix_next().

    static inline datatype *
    funcprefix_next(const colltype *coll, const datatype *prev)

      This function can be used to retrieve pointers to all elements
      in the collection. In the first call, prev should be NULL; in
      each subsequent call, prev should be the pointer returned by the
      preceding call. When there are no more elements in the
      collection the function returns NULL. The order in which
      elements are returned is undefined. Modifying the collection
      using funcprefix_insert() or funcprefix_remove() during a chain
      of calls to this function may cause some elements to be skipped
      or returned multiple times. Calling funcprefix_remove() on an
      element prior to using it as prev must not be done.

    static inline void
    funcprefix_foreach(colltype *coll, void (*func)(datatype *))

      Call func() once for each element in the collection, giving it
      pointer to the element as parameter. The order in which elements
      are traversed is undefined. Calling funcprefix_remove() for the
      element from within func() is permitted and does not affect the
      traversal of the collection. Calling funcprefix_insert() from
      within func() may cause some elements to be skipped or returned
      multiple times.
*/

#define MACSEC_COLL(colltype, funcprefix, maxelems, datatype) \
 \
typedef struct { \
  datatype datav[maxelems]; \
  MacsecCollNode nodev[maxelems]; \
  MacsecCollNode free; \
  MacsecCollNode busy; \
} colltype; \
 \
static inline void \
funcprefix ## _init(colltype *coll) \
{ \
  unsigned i; \
 \
  coll->free.next = &coll->nodev[0]; \
 \
  /* extra sizeof check to prevent Coverity warning with one-element coll */ \
  if (sizeof coll->nodev / sizeof coll->nodev[0] > 1) \
    for (i = 0; i < sizeof coll->nodev / sizeof coll->nodev[0] - 1; i++) \
      coll->nodev[i].next = &coll->nodev[i + 1]; \
 \
  coll->nodev[sizeof coll->nodev/sizeof coll->nodev[0] - 1].next = \
    &coll->free; \
 \
  coll->nodev[0].prev = &coll->free; \
 \
  for (i = 1; i < sizeof coll->nodev / sizeof coll->nodev[0]; i++) \
    coll->nodev[i].prev = &coll->nodev[i - 1]; \
 \
  coll->free.prev = \
    &coll->nodev[sizeof coll->nodev/sizeof coll->nodev[0] - 1]; \
 \
  coll->busy.next = &coll->busy; \
  coll->busy.prev = &coll->busy; \
} \
 \
static inline datatype * \
funcprefix ## _insert(colltype *coll) \
{ \
  MacsecCollNode *node = coll->free.next; \
 \
  if (node == &coll->free) \
    return (void *)0; \
 \
  node->prev->next = node->next; \
  node->next->prev = node->prev; \
 \
  coll->busy.prev->next = node; \
  node->prev = coll->busy.prev; \
 \
  coll->busy.prev = node; \
  node->next = &coll->busy; \
 \
  return &coll->datav[node - &coll->nodev[0]]; \
} \
 \
static inline void \
funcprefix ## _remove(colltype *coll, datatype *data) \
{ \
  MacsecCollNode *node = &coll->nodev[data - &coll->datav[0]]; \
 \
  node->next->prev = node->prev; \
  node->prev->next = node->next; \
 \
  coll->free.next->prev = node; \
  node->next = coll->free.next; \
 \
  coll->free.next = node; \
  node->prev = &coll->free; \
} \
 \
static inline datatype * \
funcprefix ## _next(const colltype *coll, const datatype *prev) \
{ \
  MacsecCollNode *node; \
 \
  if (!prev) \
    node = coll->busy.next; \
  else \
    node = coll->nodev[prev - &coll->datav[0]].next; \
 \
  if (node == &coll->busy) \
    return (void *)0; \
 \
  return (void *)&coll->datav[node - &coll->nodev[0]]; \
} \
 \
static inline void \
funcprefix ## _foreach(colltype *coll, void (*func)(datatype *)) \
{ \
  MacsecCollNode *node, *next; \
 \
  node = coll->busy.next; \
  while (node != &coll->busy) \
    { \
      next = node->next; \
      func(&coll->datav[node - &coll->nodev[0]]); \
      node = next; \
    } \
}

#endif /* _MACSEC_COLL_H */
