/**

   Macro for defining dictionaries (sets without duplicates) of
   objects with ordered iteration.

   File: macsec_dict.h

   @description
   A macro for defining a dictionary type and associated
   initialization, ordered iteration, insert, remove and lookup
   functions for a specific element type.

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _MACSEC_DICT_H
#define _MACSEC_DICT_H

#include "macsec_clib.h"

/** Define type and functions for a dictionary.

    @param dicttype
    Typedef name of the dictionary type to be defined.

    @param funcprefix
    Prefix that is prepended to the names of the functions to be defined.

    @maxelems
    The maximum number of elements the dictionary can hold.

    @elemtype
    The type of an element, containing a key and data, to be stored in
    the dictionary.

    @elemvar
    The name of the variable containing a pointer to the element in
    the assignment statement or in the comparison expression.

    @keytype
    The type of a key.

    @keyvar
    The name of the variable containing a pointer to the key in the
    assignment statement or in the comparison expression.

    @setstmt
    Assignment statement to copy a key pointed to by keyvar into an
    element pointed to by elemvar.

    @cmpexpr
    Comparison expression to compare an element pointed to by elemvar
    with a key pointed to by keyvar. The (integer) value of the
    expression should be negative if the key contained in the element
    is less than the key pointed to by keyvar, zero if they are equal
    and positive if the key contained in the element is greater than
    the key pointed to by keyvar.

    @description
    This macro defines a dictionary type that contains space for
    storing maxelements elements of type elemtype and associated
    dictionary management overhead. It also defines the following
    functions:

    static inline void
    funcprefix_init(dicttype *dict)

      Initialize a dictionary. This function must be called before
      using the other functions.

    static inline elemtype *
    funcprefix_insert(dicttype *dict, const keytype *keyvar)

      Insert a new element to the dictionary in the position detemined
      by the key pointed to by keyvar, copy the key into the element
      using setstmt and return pointer to the element. If no more
      elements can be inserted or if there already exists an element
      with the same key, return NULL.

    static inline elemtype *
    funcprefix_remove(dicttype *dict, const keytype *key)

      Remove the element with key key from the dictionary and return
      pointer to the removed element. If there is no element with
      matching key, return NULL. The removed element can be accessed
      using the returned pointer until the next call to
      funcprefix_insert() that may reuse the storage.

    static inline elemtype *
    funcprefix_lookup(const dicttype *dict, const keytype *key)

      Return pointer to the element with key key, or NULL if there is
      no element with matching key in the dictionary.

    static inline unsigned
    funcprefix_count(const dicttype *dict)

      Return the number of elements in the dictionary.

    static inline elemtype *
    funcprefix_first(const dicttype *dict)

      Return pointer to the element with lowest key in the dictionary,
      or NULL if there are no elements.

    static inline elemtype *
    funcprefix_last(const dicttype *dict)

      Return pointer to the element with highest key in the
      dictionary, or NULL if there are no elements.

    static inline elemtype *
    funcprefix ## _next(const dicttype *dict, const elemtype *prev)

      This function can be used to retrieve pointers to all elements
      in the dictionary. In the first call, prev should be NULL; in
      each subsequent call, prev should be the pointer returned by the
      preceding call. When there are no more elements in the
      dictionary the function returns NULL. Elements are returned in
      ascending key order. Calling funcprefix_remove() on an element
      prior to using it as prev must not be done. However, first using
      an element as prev and then removing it does not affect
      traversal of subsequent elements. Calling funcprefix_insert()
      during a chain of calls to this function may cause some elements
      to be skipped or returned multiple times.

    static inline void
    funcprefix_foreach(dicttype *coll, void (*func)(elemtype *))

      Call func() once for each element in the dictionary, giving it
      pointer to the element as parameter. Elements are traversed in
      ascending key order. Calling funcprefix_remove() for the element
      from within func() is permitted and does not affect the
      traversal of the dictionary. Calling funcprefix_insert() from
      within func() may cause some elements to be skipped or returned
      multiple times.
*/

#define MACSEC_DICT( \
  dicttype, funcprefix, maxelems, \
  elemtype, elemvar, keytype, keyvar, \
  setstmt, cmpexpr) \
 \
typedef struct { \
  elemtype elemv[maxelems]; \
  elemtype *nextv[maxelems]; \
  unsigned char idxv[maxelems]; \
  unsigned idxc; \
} dicttype; \
 \
static inline int \
funcprefix ## _bsearch( \
  const dicttype *dict, const keytype *keyvar, unsigned *dictpos) \
{ \
  const elemtype *elemvar; \
  unsigned dictleft, dictright, dictcenter; \
  int dictdiff; \
 \
  dictleft = 0; \
  dictright = dict->idxc; \
  dictcenter = (dictleft + dictright) >> 1; \
 \
  while (dictright > dictleft) \
    { \
      elemvar = &dict->elemv[dict->idxv[dictcenter]]; \
 \
      if (!(dictdiff = (cmpexpr))) \
        { \
          if (dictpos) \
            *dictpos = dictcenter; \
          return 1; \
        } \
 \
      if (dictdiff < 0) \
        dictleft = dictcenter + 1; \
      else \
        dictright = dictcenter; \
 \
      dictcenter = (dictleft + dictright) >> 1; \
    } \
 \
  if (dictpos) \
    *dictpos = dictcenter; \
  return 0; \
} \
 \
static inline void \
funcprefix ## _init(dicttype *dict) \
{ \
  unsigned i; \
 \
  for (i = 0; i < sizeof dict->idxv / sizeof dict->idxv[0]; i++) \
    { \
      dict->nextv[i] = (void *)0; \
      dict->idxv[i] = (unsigned char)i; \
    } \
 \
  dict->idxc = 0; \
} \
 \
static inline elemtype * \
funcprefix ## _insert(dicttype *dict, const keytype *keyvar) \
{ \
  elemtype *elemvar; \
  unsigned char dictnew; \
  unsigned dictpos; \
 \
  if (dict->idxc >= sizeof dict->idxv / sizeof dict->idxv[0]) \
    return (void *)0; \
 \
  if (funcprefix ## _bsearch(dict, keyvar, &dictpos)) \
    return (void *)0; \
 \
  dictnew = dict->idxv[dict->idxc]; \
  memmove( \
    &dict->idxv[dictpos + 1], \
    &dict->idxv[dictpos], \
    (dict->idxc - dictpos) * sizeof dict->idxv[0]); \
  dict->idxv[dictpos] = dictnew; \
  dict->idxc++; \
 \
  if (dictpos > 0) \
    dict->nextv[dict->idxv[dictpos - 1]] = &dict->elemv[dictnew]; \
  if (dictpos + 1 < dict->idxc) \
    dict->nextv[dictnew] = &dict->elemv[dict->idxv[dictpos + 1]]; \
 \
  elemvar = &dict->elemv[dictnew]; \
  setstmt; \
  return elemvar; \
} \
 \
static inline elemtype * \
funcprefix ## _remove(dicttype *dict, const keytype *key) \
{ \
  unsigned char old; \
  unsigned pos; \
 \
  if (!funcprefix ## _bsearch(dict, key, &pos)) \
    return (void *)0; \
 \
  old = dict->idxv[pos]; \
  dict->idxc--; \
  memmove( \
    &dict->idxv[pos], \
    &dict->idxv[pos + 1], \
    (dict->idxc - pos) * sizeof dict->idxv[0]); \
  dict->idxv[dict->idxc] = old; \
 \
  /* sizeof check below prevents Coverity warning with one-element dict */ \
  if (sizeof dict->idxv / sizeof dict->idxv[0] > 1 && pos > 0) \
    { \
      if (pos < dict->idxc) \
        dict->nextv[dict->idxv[pos - 1]] = &dict->elemv[dict->idxv[pos]]; \
      else \
        dict->nextv[dict->idxv[pos - 1]] = (void *)0; \
    } \
 \
  dict->nextv[old] = (void *)0; \
 \
  return &dict->elemv[old]; \
} \
 \
static inline elemtype * \
funcprefix ## _lookup(const dicttype *dict, const keytype *key) \
{ \
  unsigned pos; \
 \
  if (!funcprefix ## _bsearch(dict, key, &pos)) \
    return (void *)0; \
 \
  return (void *)&dict->elemv[dict->idxv[pos]]; \
} \
 \
static inline unsigned \
funcprefix ## _count(const dicttype *dict) \
{ \
  return dict->idxc; \
} \
 \
static inline elemtype * \
funcprefix ## _first(const dicttype *dict) \
{ \
  if (dict->idxc > 0) \
    return (void *)&dict->elemv[dict->idxv[0]]; \
  else \
    return (void *)0; \
} \
 \
static inline elemtype * \
funcprefix ## _last(const dicttype *dict) \
{ \
  if (dict->idxc > 0) \
    return (void *)&dict->elemv[dict->idxv[dict->idxc - 1]]; \
  else \
    return (void *)0; \
} \
 \
static inline elemtype * \
funcprefix ## _next(const dicttype *dict, const elemtype *prev) \
{ \
  if (!prev) \
    { \
      if (dict->idxc > 0) \
        return (void *)&dict->elemv[dict->idxv[0]]; \
      else \
        return (void *)0; \
    } \
 \
  return dict->nextv[prev - &dict->elemv[0]]; \
} \
 \
static inline void \
funcprefix ## _foreach(dicttype *dict, void (*func)(elemtype *)) \
{ \
  elemtype *elem, *next, *tmp; \
 \
  if (dict->idxc <= 0) \
    return; \
 \
  elem = &dict->elemv[dict->idxv[0]]; \
 \
  while (elem) \
    { \
      next = dict->nextv[elem - &dict->elemv[0]]; \
      func(elem); \
      tmp = &dict->elemv[dict->idxv[0]]; \
      while (tmp) \
        { \
          if (tmp == next) \
            break; \
          tmp = dict->nextv[tmp - &dict->elemv[0]]; \
        } \
      if (tmp == NULL) \
        next = dict->nextv[elem - &dict->elemv[0]]; \
      elem = next; \
    } \
}

#endif /* _MACSEC_DICT_H */
