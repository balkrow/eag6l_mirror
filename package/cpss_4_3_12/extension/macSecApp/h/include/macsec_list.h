/**

   Linked list.

   File: macsec_list.h

   @description
   Doubly linked list.

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _MACSEC_LIST_H
#define _MACSEC_LIST_H

#ifndef inline
#define inline __inline
#endif

/** List link. */
typedef struct MacsecListLink {
  /* Pointer to the next list link. */
  struct MacsecListLink *prev;
  /* Pointer to the previous list link. */
  struct MacsecListLink *next;
} MacsecListLink;

/** Initialize a list. This function must be called before calling the
    other functions. */
static inline void
macsec_list_init(MacsecListLink *list)
{
  list->prev = list;
  list->next = list;
}

/** Insert a new link link before an existing link next. */
static inline void
macsec_list_insbefore(MacsecListLink *next, MacsecListLink *link)
{
  next->prev->next = link;
  link->next = next;

  link->prev = next->prev;
  next->prev = link;
}

/** Insert a new link link after an existing link prev. */
static inline void
macsec_list_insafter(MacsecListLink *prev, MacsecListLink *link)
{
  prev->next->prev = link;
  link->prev = prev;

  link->next = prev->next;
  prev->next = link;
}

/** Remove a link from the list it was inserted into. */
static inline void
macsec_list_remove(MacsecListLink *link)
{
  link->next->prev = link->prev;
  link->prev->next = link->next;

  link->prev = (void *)0;
  link->next = (void *)0;
}

/** Insert a new link link in the end of list list. */
static inline void
macsec_list_append(MacsecListLink *list, MacsecListLink *link)
{
  macsec_list_insbefore(list, link);
}

/** Insert a new link link in the beginning of list list. */
static inline void
macsec_list_prepend(MacsecListLink *list, MacsecListLink *link)
{
  macsec_list_insafter(list, link);
}

/** Return pointer to the next containing object in the list list. If
    prev is NULL, return the first containing object. If prev is
    non-null, return pointer to the next containing object. In either
    case, return NULL if there are no more objects in the list. The
    position of the appropriate list link within each containing
    object is given by offset. */
static inline void *
macsec_list_next(const MacsecListLink *list, unsigned offset, const void *prev)
{
  const MacsecListLink *link;

  if (!prev)
    link = list;
  else
    link = (void *)((unsigned char *)prev + offset);

  link = link->next;

  if (link == list)
    return (void *)0;
  else
    return (void *)((unsigned char *)link - offset);
}

/** Call func() for each containing object in the list. The position
    of the appropriate list link within each containing object is
    given by offset. Removal of the link of the object given to func()
    from within func() using macsec_list_remove() is permitted. */
static inline void
macsec_list_foreach(MacsecListLink *list, unsigned offset, void *func)
{
  MacsecListLink *link, *next;

  link = list->next;
  while (link != list)
    {
      next = link->next;
      ((void (*)(void *))func)((unsigned char *)link - offset);
      link = next;
    }
}

/** Return nonzero if the list list is empty, zero otherwise. */
static inline int
macsec_list_empty(const MacsecListLink *list)
{
  return list->next == list;
}

#endif /* _MACSEC_LIST_H */
