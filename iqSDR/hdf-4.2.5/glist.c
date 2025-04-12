/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at      *
 * http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/************************************************************************
  Credits:
          Original code is part of the public domain 'Generic List Library'
          by Keith Pomakis(kppomaki@jeeves.uwaterloo.ca)-Spring, 1994
          I modified it to adhere to HDF coding standards.

  1996/06/04 - George V. 
 ************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Id: glist.c 4932 2007-09-07 17:17:23Z bmribler $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include "glist.h"

/* Commmented out the following */
#if 0
/* Macintosh */
#if defined SYMANTEC_C || defined macintosh
#define malloc NewPtr
#define free   DisposePtr
#endif /* SYMANTEC_C || macintosh */

#define NEW(x) ((x *) emalloc(sizeof(x)))
static void *emalloc(unsigned int n);
static char *module = "generic_list";
#endif /* Commented out */

/*+
****************************************************************************
GENERAL
****************************************************************************

A set of basic generic doubly-linked list functions were designed and
programmed first (along with a suitable efficient data structure), and then
some higher-level functions were added to increase ease of use.  The
functionality of stacks, queues and sorted lists were then added.  In
actuality, these functions (with the exception of one of the sorted-list
functions) are nothing more than aliases for the appropriate generic list
operations.  This aliasing is behind the scenes, however, and the user of
this library may treat the operation of lists, stacks and queues in this
library as completely separate functionality.

In order to make the library completely generic, it was designed to
manipulate pointers of type void *.  Therefore, it is assumed that the
programmer is statically or dynamically creating the objects of interest,
and using the generic list functions to manipulate them.  It is up to the
programmer to handle the allocation and deallocation of the memory for the
objects themselves.

A pointer to the same object may be stored in a list multiple times.  The
only restriction imposed is that a NULL pointer may not be stored.


****************************************************************************
USAGE
****************************************************************************

The use of this library is simple and straight-forward.  In every source
file that requires the use of generic list functions, the line:

#include "glist.h"

must be included at the top of the file.  For those who hand-craft their own
makefiles, "generic_list.h" should become a prerequisite for each of these
files, as well as for "generic_list.c" itself.

The library defines three data types:

    Generic_list
    Generic_stack
    Generic_queue

The usage of these functions is best illustrated with an example:
Error checking of the return values from the functions are not shown.

foo() {
    Generic_stack stack;
    My_object *obj;

    HDGSinitialize_stack(&stack);

    obj = new_object();
    HDGSpush(stack, obj);
    ...
    obj = HDGSpop(stack);
    free(obj);
    ...
    HDGSdestroy_stack(&stack);
}

Each list must be initialized before use and should be destroyed after it is
no longer needed.  The programmer must handle the allocation and
deallocation of the memory for the objects being stored.  Explicit memory
management for the lists is not necessary.

****************************************************************************
LIST OF FUNCTIONS
****************************************************************************

The following are the headers of the functions provided in the generic list
library.  They are described in more detail later.

Generic Lists
-------------

intn HDGLinitialize_list(Generic_list *list);
void HDGLdestroy_list(Generic_list *list);
intn HDGLadd_to_beginning(Generic_list list, void *pointer);
intn HDGLadd_to_end(Generic_list list, void *pointer);
intn HDGLadd_to_list(Generic_list list, void *pointer);
void *HDGLremove_from_beginning(Generic_list list);
void *HDGLremove_from_end(Generic_list list);
void *HDGLremove_from_list(Generic_list list, void *pointer);
void HDGLremove_all(Generic_list list);
void *HDGLpeek_at_beginning(Generic_list list);
void *HDGLpeek_at_end(Generic_list list);

void *HDGLfirst_in_list(Generic_list list);
void *HDGLnext_in_list(Generic_list list);
void *HDGLcurrent_in_list(Generic_list list);
void *HDGLremove_current(Generic_list list);
void *HDGLprevious_in_list(Generic_list list);
void *HDGLlast_in_list(Generic_list list);
void HDGLreset_to_beginning(Generic_list list);
void HDGLreset_to_end(Generic_list list);

intn HDGLnum_of_objects(Generic_list list);
intn HDGLis_empty(Generic_list list);
intn HDGLis_in_list(Generic_list list, void *pointer);
Generic_list HDGLcopy_list(Generic_list list);

void HDGLperform_on_list
     (Generic_list list, void (*fn)(void *pointer, void *args), void *args);
void *HDGLfirst_that
     (Generic_list list, intn (*fn)(void *pointer, void *args), void *args);
void *HDGLnext_that
     (Generic_list list, intn (*fn)(void *pointer, void *args), void *args);
void *HDGLprevious_that
     (Generic_list list, intn (*fn)(void *pointer, void *args), void *args);
void *HDGLlast_that
     (Generic_list list, intn (*fn)(void *pointer, void *args), void *args);
Generic_list HDGLall_such_that
     (Generic_list list, intn (*fn)(void *pointer, void *args), void *args);
void HDGLremove_all_such_that
     (Generic_list list, intn (*fn)(void *pointer, void *args), void *args);


Generic Sorted Lists
--------------------

intn HDGLinitialize_sorted_list(Generic_list *list, int (*lt)(void *a, void *b));

...and all Generic_list functions EXCEPT:

intn HDGLadd_to_beginning(Generic_list list, void *pointer);
intn HDGLadd_to_end(Generic_list list, void *pointer);
void *HDGLremove_from_beginning(Generic_list list);
void *HDGLremove_from_end(Generic_list list);


Generic Stacks(HDGSxxx)
----------------------

intn HDGSinitialize_stack(Generic_stack *stack);
void HDGSdestroy_stack(Generic_stack *stack);
intn HDGSpush(Generic_stack stack, void *pointer);
void *HDGSpop(Generic_stack stack);
void HDGSpop_all(Generic_stack stack);
void *HDGSpeek_at_top(Generic_stack stack);
Generic_stack HDGScopy_stack(Generic_stack stack);

* This is a list fcn *
intn HDGLis_empty(Generic_stack stack); 

Generic Queues(HDGQxxx)
----------------------

intn HDGQinitialize_queue(Generic_queue *queue);
void HDGQdestroy_queue(Generic_queue *queue);
intn HDGQenqueue(Generic_queue queue, void *pointer);
void *HDGQdequeue(Generic_queue queue);
void HDGQdequeue_all(Generic_queue queue);
void *HDGQpeek_at_head(Generic_queue queue);
void *HDGQpeek_at_tail(Generic_queue queue);
Generic_queue HDGQcopy_queue(Generic_queue queue);

* This is a list fcn *
intn HDGLis_empty(Generic_queue queue);

****************************************************************************
HINTS
****************************************************************************

Technically, any of the above functions can be used with any of the three
data types.  For example, one can use HDGLperform_on_list() to perform a
specified function on every object in a queue, or HDGLis_in_list() to determine
whether or not a particular object is a member of a stack.  One can even
HDGSpop from a queue and HDGQdequeue from a stack.  However, such usage is not
recommended, as it is contrary to the logical usage of such data
structures.

 +*/

/******************************************************************************
 NAME
     HDGLinitialize_list
 DESCRIPTION
     Every list must be initialized before it is used.  The only time it is
     valid to re-initialize a list is after it has been destroyed.
 RETURNS
     SUCCEED/FAIL
*******************************************************************************/
intn
HDGLinitialize_list(Generic_list *list)
{
    CONSTR(FUNC, "HDGLinitialize_list");	/* for HERROR */
    intn  ret_value = SUCCEED;

    /* Allocate an intialize info struct */
    list->info = (Generic_list_info *)HDmalloc(sizeof(Generic_list_info));

    if (list->info != NULL)
      {
          list->info->pre_element.pointer  = NULL;
          list->info->pre_element.previous = &list->info->pre_element;
          list->info->pre_element.next     = &list->info->post_element;
          list->info->post_element.pointer = NULL;
          list->info->post_element.previous = &list->info->pre_element;
          list->info->post_element.next     = &list->info->post_element;

          list->info->current = &list->info->pre_element;
          list->info->deleted_element.pointer = NULL;
          list->info->lt      = NULL;
          list->info->num_of_elements = 0;
      }
    else
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

done:
    if (ret_value == FAIL)
      {
      }

return ret_value;
} /* HDGLinitialize_list() */

/******************************************************************************
 NAME
     HDGLinitialize_sorted_list
 DESCRIPTION
    This function initializes a sorted list.  A less-than function must be
    specified which accepts two pointers, a and b, and returns TRUE
    (non-zero) if a is less than b, FALSE otherwise.

    Once a list is initialized in this way, all of the generic list
    functions described above can be used, except for:

        void HDGLadd_to_beginning(Generic_list list, void *pointer);
        void HDGLadd_to_end(Generic_list list, void *pointer);
        void *HDGLremove_from_beginning(Generic_list list);
        void *HDGLremove_from_end(Generic_list list);

    and the list will remain sorted by the criteria specified by the
    less-than function.  The only time it is valid to re-initialize a list
    is after it has been destroyed.
 RETURNS
     SUCEED/FAIL
*******************************************************************************/
intn
HDGLinitialize_sorted_list(Generic_list *list, 
                       intn (*lt)(VOIDP /* a */, VOIDP /* b */))
{
    intn ret_value = SUCCEED;

    /* First initialize list */
    if ((ret_value = HDGLinitialize_list(list)) == FAIL)
        goto done;
    else
        list->info->lt = lt; /* Set sort fcn */

done:
    if (ret_value == FAIL)
      {
      }

return ret_value;
} /* HDGLinitialize_sorted_list() */

/******************************************************************************
 NAME
     HDGLdestroy_list
 DESCRIPTION
    When a list is no longer needed, it should be destroyed.  This process
    will automatically remove all remaining objects from the list.  However,
    the memory for these objects will not be reclaimed, so if the objects
    have no other references, care should be taken to purge the list and
    free all objects before destroying the list.

    It is an error to destroy a list more than once (unless it has been
    re-initialized in the meantime).
 RETURNS
     Nothing
*******************************************************************************/
void
HDGLdestroy_list(Generic_list *list)
{
    /* Fist remove all nodes */
    HDGLremove_all(*list);

    /* Free the info struct last */
    HDfree((VOIDP)list->info);
} /* HDGLdestroy_list() */

/******************************************************************************
 NAME
    HDGLadd_to_beginning
 DESCRIPTION
    This function will add the specified object to the beginning of the
    list.  The pointer must not be NULL.
 RETURNS
    SUCCEED/FAIL
*******************************************************************************/
intn
HDGLadd_to_beginning(Generic_list list, 
                 VOIDP pointer)
{
    CONSTR(FUNC, "HDGLadd_to_beginning");	/* for HERROR */
    Generic_list_element *element;
    intn   ret_value = SUCCEED;

    /* Check data element */
    if (pointer == NULL)
        HGOTO_ERROR(DFE_ARGS,FAIL); 

    /* Allocate and add to beginning of list */
    element = (Generic_list_element *)HDmalloc(sizeof(Generic_list_element));
    if (element != NULL)
      {
          element->next     = list.info->pre_element.next;
          element->previous = &list.info->pre_element;
          element->pointer  = pointer;

          list.info->pre_element.next->previous = element;
          list.info->pre_element.next = element;

          list.info->num_of_elements++;
      }
    else
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

done:
    if (ret_value == FAIL)
      {
      }

return ret_value;
} /* HDGLadd_to_beginning() */

/******************************************************************************
 NAME
     HDGLadd_to_end
 DESCRIPTION
    This function will add the specified object to the end of the
    list.  The pointer must not be NULL.
 RETURNS
    SUCCEED/FAIL
*******************************************************************************/
intn
HDGLadd_to_end(Generic_list list, 
           VOIDP pointer)
{
    CONSTR(FUNC, "HDGLadd_to_end");	/* for HERROR */
    Generic_list_element *element;
    intn   ret_value = SUCCEED;

    /* Check data element */
    if (pointer == NULL) 
        HGOTO_ERROR(DFE_ARGS,FAIL);

    /* Allocate and add to end of list */
    element = (Generic_list_element *)HDmalloc(sizeof(Generic_list_element));
    if (element != NULL)
      {
          element->next     = &list.info->post_element;
          element->previous = list.info->post_element.previous;
          element->pointer  = pointer;

          list.info->post_element.previous->next = element;
          list.info->post_element.previous = element;

          list.info->num_of_elements++;
      }
    else
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

done:
    if (ret_value == FAIL)
      {
      }

return ret_value;
} /* HDGLadd_to_end() */

/******************************************************************************
 NAME
     HDGLadd_to_list
 DESCRIPTION
    This function will add the specified object to the list.  The pointer
    must not be NULL.
 RETURNS
     SUCCEED/FAIL
*******************************************************************************/
intn
HDGLadd_to_list(Generic_list list, 
            VOIDP pointer)
{
    CONSTR(FUNC, "HDGLadd_to_list");	/* for HERROR */
    Generic_list_element *element, *new_element;
    intn ret_value = SUCCEED;

    /* Check to see if there is a sort fcn */
    if (list.info->lt) 
      {
          /* Check data element */
        if (pointer == NULL) 
            HGOTO_ERROR(DFE_ARGS,FAIL);

        element = list.info->pre_element.next;
        while (element != &list.info->post_element &&
                (*list.info->lt)(element->pointer, pointer))
            element = element->next;
    
       /* Allocate and add to list */
        new_element = (Generic_list_element *)HDmalloc(sizeof(Generic_list_element));
        if (new_element != NULL)
          {
              new_element->next     = element;
              new_element->previous = element->previous;
              new_element->pointer  = pointer;

              element->previous->next = new_element;
              element->previous = new_element;

              list.info->num_of_elements++;
          }
        else
            HGOTO_ERROR(DFE_NOSPACE, FAIL);
      }
    else /* no sort fcn so add to end of list */
        ret_value = HDGLadd_to_end(list, pointer);

done:
    if (ret_value == FAIL)
      {
      }

return ret_value;
} /* HDGLadd_to_list() */

/******************************************************************************
 NAME
     HDGLremove_from_list
 DESCRIPTION
    This function will remove the specified object from the list and return
    it.  If the specified object does not exist in the list, NULL is
    returned.  If the specified object exists in the list more than once,
    only the last reference to it is removed.

 RETURNS
    Element removed if successful and NULL otherwise
*******************************************************************************/
VOIDP
HDGLremove_from_list(Generic_list list, 
                 VOIDP pointer)
{
    Generic_list_element *element;
    VOIDP ret_value = NULL;

    /* Find element in list */
    element = list.info->post_element.previous;
    while (element != &list.info->pre_element && element->pointer != pointer)
        element = element->previous;

    if (element == &list.info->pre_element)
      { /* No such element was found. */
        ret_value = NULL;
        goto done;
      }

    /* Have found element */
    if (element == list.info->current) 
      {
        list.info->deleted_element.previous = element->previous;
        list.info->deleted_element.next     = element->next;
        list.info->current                  = &list.info->deleted_element;
      }

    element->previous->next = element->next;
    element->next->previous = element->previous;

    HDfree(element); /* free element */
    list.info->num_of_elements--;

    ret_value = pointer; /* return ptr to original element */

done:
    if (ret_value == NULL)
      {
      }

return ret_value;
} /* HDGLremove_from_list() */

/******************************************************************************
 NAME
     HDGLremove_from_beginning
 DESCRIPTION
    This function will remove the first object from the beginning of the
    list and return it.  If the list is empty, NULL is returned.
 RETURNS
    First Element if successful and NULL otherwise.
*******************************************************************************/
VOIDP
HDGLremove_from_beginning(Generic_list list)
{
    Generic_list_element *element;
    VOIDP pointer;
    VOIDP ret_value = NULL;

    /* Check to see if there any elements in the list */
    if(list.info->num_of_elements == 0)
      { /* nope */
        ret_value = NULL;
        goto done;
      }

    /* Remove first element */
    element = list.info->pre_element.next;
    if (element == list.info->current)
        list.info->current = &list.info->pre_element;

    pointer = element->pointer;
    list.info->pre_element.next = element->next;
    element->next->previous = &list.info->pre_element;

    HDfree(element);
    list.info->num_of_elements--;

    ret_value = pointer; /* return the fist element */

done:
    if (ret_value == NULL)
      {
      }

return ret_value;
} /* HDGLremove_from_beginning() */

/******************************************************************************
 NAME
     HDGLremove_from_end
 DESCRIPTION
    This function will remove the last object from the end of the list and
    return it.  If the list is empty, NULL is returned.
 RETURNS
    Last element if successfull and NULL otherwise
*******************************************************************************/
VOIDP
HDGLremove_from_end(Generic_list list)
{
    Generic_list_element *element;
    VOIDP pointer;
    VOIDP ret_value = NULL;

    /* Check to see if there any elements in the list */
    if(list.info->num_of_elements == 0)
      { /* nope */
        ret_value = NULL;
        goto done;
      }

    element = list.info->post_element.previous;
    if (element == list.info->current)
        list.info->current = &list.info->post_element;

    pointer = element->pointer;
    list.info->post_element.previous = element->previous;
    element->previous->next = &list.info->post_element;

    HDfree(element);
    list.info->num_of_elements--;

    ret_value = pointer; /* return last element */

done:
    if (ret_value == NULL)
      {
      }

return ret_value;
} /* HDGLremove_from_end() */

/******************************************************************************
 NAME
     HDGLremove_current
 DESCRIPTION
    This function will remove the current object from the list and return
    it.  If the current object has already been removed, if current points
    to the beginning or end of the list, or if the list is empty, NULL is
    returned.
 RETURNS
    Current element if successful and NULL otherwise.
*******************************************************************************/
VOIDP
HDGLremove_current(Generic_list list)
{
    Generic_list_element *element;
    VOIDP pointer;
    VOIDP ret_value = NULL;

    element = list.info->current;
    if (element->pointer == NULL)
      { /* current is empty */
        ret_value = NULL;
        goto done;
      }

    list.info->deleted_element.previous = element->previous;
    list.info->deleted_element.next     = element->next;
    list.info->current                  = &list.info->deleted_element;

    pointer = element->pointer;
    element->next->previous = element->previous;
    element->previous->next = element->next;

    HDfree(element);
    list.info->num_of_elements--;

    ret_value = pointer; /* return current element */

done:
    if (ret_value == NULL)
      {
      }

return ret_value;
} /* HDGLremove_current() */

/******************************************************************************
 NAME
     HDGLremove_all
 DESCRIPTION
    This function will remove all objects from the list.  Note that the
    memory for these objects will not be reclaimed, so if the objects have
    no other references, it is best to avoid this function and remove the
    objects one by one, freeing them when necessary.
 RETURNS
    Nothing
*******************************************************************************/
void
HDGLremove_all(Generic_list list)
{
    Generic_list_element *element;

    /* remove all the elements from the list */
    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        element = element->next;
        HDfree(element->previous);
      }

    list.info->pre_element.next = &list.info->post_element;
    list.info->post_element.previous = &list.info->pre_element;
    list.info->num_of_elements = 0;
} /* HDGLremove_all() */

/******************************************************************************
 NAME
     HDGLpeek_at_beginning
 DESCRIPTION
    This function will return the first object in the list.  If the list is
    empty, NULL is returned.
 RETURNS
    First element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLpeek_at_beginning(Generic_list list)
{
    return list.info->pre_element.next->pointer;
} /* HDGLpeek_at_beginning() */

/******************************************************************************
 NAME
     HDGLpeek_at_end
 DESCRIPTION
    This function will return the last object in the list.  If the list is
    empty, NULL is returned.
 RETURNS
    Last element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLpeek_at_end(Generic_list list)
{
    return list.info->post_element.previous->pointer;
} /* HDGLpeek_at_end() */

/******************************************************************************
 NAME
     HDGLfirst_in_list
 DESCRIPTION
    This function will return the first object in the list and mark it as
    the current object.  If the list is empty, NULL is returned.
 RETURNS
    First element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLfirst_in_list(Generic_list list)
{
    list.info->current = list.info->pre_element.next->next->previous;
    return list.info->current->pointer;
} /* HDGLfirst_in_list() */

/******************************************************************************
 NAME
     HDGLcurrent_in_list
 DESCRIPTION
    This function will return the object in the list that is considered
    the current object (as defined by the surrounding functions).  If the
    current object has just been removed, if current points to the
    beginning or end of the list, or if the list is empty, NULL is
    returned.
 RETURNS
    Current element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLcurrent_in_list(Generic_list list)
{
    return list.info->current->pointer;
} /* HDGLcurrent_in_list() */

/******************************************************************************
 NAME
     HDGLlast_in_list
 DESCRIPTION
    This function will return the last object in the list and mark it as
    the current object.  If the list is empty, NULL is returned.
 RETURNS
    Last element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLlast_in_list(Generic_list list)
{
    list.info->current = list.info->post_element.previous->previous->next;
    return list.info->current->pointer;
} /* HDGLlast_in_list() */

/******************************************************************************
 NAME
     HDGLnext_in_list
 DESCRIPTION
    This function will return the next object in the list and mark it as
    the current object.  If the end of the list is reached, or if the list
    is empty, NULL is returned.
 RETURNS
    Next element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLnext_in_list(Generic_list list)
{
    list.info->current = list.info->current->next;
    return list.info->current->pointer;
} /* HDGLnext_in_list() */

/******************************************************************************
 NAME
     HDGLprevious_in_list
 DESCRIPTION
    This function will return the previous object in the list and mark it
    as the current object.  If the beginning of the list is reached, or if
    the list is empty, NULL is returned.
 RETURNS
    Previous element in list if non-empty, otherwise NULL.
*******************************************************************************/
VOIDP
HDGLprevious_in_list(Generic_list list)
{
    list.info->current = list.info->current->previous;
    return list.info->current->pointer;
} /* HDGLprevious_in_list() */

/******************************************************************************
 NAME
     HDGLreset_to_beginning
 DESCRIPTION
    This function will reset the list to the beginning.  Therefore, current
    points to the beginning of the list, and the next object in the list is
    the first object.
 RETURNS
     Nothing
*******************************************************************************/
void
HDGLreset_to_beginning(Generic_list list)
{
    list.info->current = &list.info->pre_element;
} /* HDGLreset_to_beginning() */

/******************************************************************************
 NAME
     HDGLreset_to_end
 DESCRIPTION
    This function will reset the list to the end.  Therefore, current
    points to the end of the list, and the previous object in the list is
    the last object.
 RETURNS
     Nothing
*******************************************************************************/
void
HDGLreset_to_end(Generic_list list)
{
    list.info->current = &list.info->post_element;
} /* rest_to_end() */

/******************************************************************************
 NAME
     HDGLnum_of_objects
 DESCRIPTION
    This function will determine the number of objects in the list.
 RETURNS
    Number of objects in list
*******************************************************************************/
intn
HDGLnum_of_objects(Generic_list list)
{
    return (intn)list.info->num_of_elements;
} /* HDGLnum_of_objects() */

/******************************************************************************
 NAME
     HDGLis_empty
 DESCRIPTION
    Finds if list is empty
 RETURNS
    This function will return TRUE (1) if the list is empty, and FALSE (0)
    otherwise.
*******************************************************************************/
intn
HDGLis_empty(Generic_list list)
{
    return (list.info->num_of_elements == 0);
} /* HDGLis_empty() */

/******************************************************************************
 NAME
     HDGLis_in_list
 DESCRIPTION
     Detemines if the object is in the list.
 RETURNS
    This function will return TRUE (1) if the specified object is a member
    of the list, and FALSE (0) otherwise.
*******************************************************************************/
intn
HDGLis_in_list(Generic_list list, 
           VOIDP pointer)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;

    while (element != &list.info->post_element && element->pointer != pointer)
        element = element->next;

    return (element != &list.info->post_element);
} /* HDGLis_in_list() */

/******************************************************************************
 NAME
     HDGLcopy_list
 DESCRIPTION
    This function will make a copy of the list.  The objects themselves
    are not copied; only new references to them are made.  The new list
    loses its concept of the current object.
 RETURNS
    A copy of the orginal list.
*******************************************************************************/
Generic_list
HDGLcopy_list(Generic_list list)
{
    Generic_list list_copy;
    Generic_list_element *element;
    intn  ret_value = SUCCEED;

    list_copy.info = NULL; /* intialize info to NULL */

    /* initialize new list */
    if (HDGLinitialize_sorted_list(&list_copy, list.info->lt) == FAIL)
      {
        ret_value = FAIL;
        goto done;
      }

    /* copy over every element to new list */
    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        if (HDGLadd_to_end(list_copy, element->pointer) == FAIL)
            {
                ret_value = FAIL;
                break;
            }
        element = element->next;
      }

done:    
    if (ret_value == FAIL)
      { /* need to remove all elements from copy */
          if (list_copy.info != NULL)
            {
                HDGLremove_all(list_copy);
            }

          list_copy.info = NULL; /* set to NULL */
      }

    return list_copy;
} /* HDGLcopy_list() */

/******************************************************************************
 NAME
     HDGLperform_on_list
 DESCRIPTION
    This function will perform the specified function on each object in the
    list.  Any optional arguments required can be passed through args.
 RETURNS
    Nothing
*******************************************************************************/
void
HDGLperform_on_list(Generic_list list, 
                void (*fn)(VOIDP /* pointer */, VOIDP /* args */),
                VOIDP args)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      { /* call fcn on each element */
        (*fn)(element->pointer, args);
        element = element->next;
      }
} /* HDGLperform_on_list() */

/******************************************************************************
 NAME
     HDGLfirst_that
 DESCRIPTION
     This function will find and return the first object in the list which
     causes the specified function to return a TRUE (non-zero) value.  Any
     optional arguments required can be passed through args.  The found
     object is then marked as the current object.  If no objects in the list
     meet the criteria of the specified function, NULL is returned.
 RETURNS
     Element if successful and NULL otherwise.
*******************************************************************************/
VOIDP
HDGLfirst_that(Generic_list list, 
           intn (*fn)(VOIDP /* pointer */, VOIDP /* args */), 
           VOIDP args)
{
    Generic_list_element *element;

    element = list.info->pre_element.next;
    while (element != &list.info->post_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->next;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
} /* HDGLfirst_that() */

/******************************************************************************
 NAME
     HDGLnext_that
 DESCRIPTION
     This function will find and return the next object in the list which
     causes the specified function to return a TRUE (non-zero) value.  Any
     optional arguments required can be passed through args.  The found
     object is then marked as the current object.  If there are no objects
     left in the list that meet the criteria of the specified function,
     NULL is returned.
 RETURNS
     Element if successful and NULL otherwise.
*******************************************************************************/
VOIDP
HDGLnext_that(Generic_list list, 
          intn (*fn)(VOIDP /* pointer */, VOIDP /* args */), 
          VOIDP args)
{
    Generic_list_element *element;

    element = list.info->current->next;
    while (element != &list.info->post_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->next;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
} /* HDGLnext_that() */

/******************************************************************************
 NAME
     HDGLprevious_that
 DESCRIPTION
     This function will find and return the previous object in the list
     which causes the specified function to return a TRUE (non-zero) value.
     Any optional arguments required can be passed through args.  The found
     object is then marked as the current object.  If there are no objects
     left in the list that meet the criteria of the specified function,
     NULL is returned.
 RETURNS
     Element if successful and NULL otherwise.
*******************************************************************************/
VOIDP
HDGLprevious_that(Generic_list list, 
              intn (*fn)(VOIDP /* pointer */, VOIDP /* args */), 
              VOIDP args)
{
    Generic_list_element *element;

    element = list.info->current->previous;
    while (element != &list.info->pre_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->previous;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
} /* HDGLprevious_that() */

/******************************************************************************
 NAME
     HDGLlast_that
 DESCRIPTION
     This function will find and return the last object in the list which
     causes the specified function to return a TRUE (non-zero) value.  Any
     optional arguments required can be passed through args.  The found
     object is then marked as the current object.  If no objects in the
     list meet the criteria of the specified function, NULL is returned.
 RETURNS
     Element if successful and NULL otherwise.
*******************************************************************************/
VOIDP
HDGLlast_that(Generic_list list, 
          intn (*fn)(VOIDP /* pointer */, VOIDP /* args */), 
          VOIDP args)
{
    Generic_list_element *element;

    element = list.info->post_element.previous;
    while (element != &list.info->pre_element &&
                            !(*fn)(element->pointer, args)) 
      {
        element = element->previous;
      }

    if (element->pointer)
        list.info->current = element;

    return element->pointer;
} /* HDGLlast_that() */

/******************************************************************************
 NAME
    HDGLall_such_that
 DESCRIPTION
    This function will return a new list containing all of the objects in
    the specified list which cause the specified function to return a TRUE
    (non-zero) value.  Any optional arguments required can be passed
    through args. The objects themselves are not copied; only new
    references to them are made.
 RETURNS
    New list if successful and empty if not.
*******************************************************************************/
Generic_list
HDGLall_such_that(Generic_list list, 
              intn (*fn)(VOIDP /* pointer */, VOIDP /* args */), 
              VOIDP args)
{
    Generic_list list_copy;
    Generic_list_element *element;
    intn  ret_value = SUCCEED;

    /* initialize copy of list */
    if (HDGLinitialize_sorted_list(&list_copy, list.info->lt) == FAIL)
      {
        ret_value = FAIL;
        goto done;
      }

    /* copy over elments that satisfy the fcn */
    element = list.info->pre_element.next;
    while (element != &list.info->post_element) 
      {
        if ((*fn)(element->pointer, args))
          {
            if (HDGLadd_to_end(list_copy, element->pointer) == FAIL)
              {
                  ret_value = FAIL;
                  break;
              }
          }
        element = element->next;
      }
    
done:    
    if (ret_value == FAIL)
      {
          if (list_copy.info != NULL)
            {
                HDGLremove_all(list_copy);
            }

          list_copy.info = NULL; /* set to NULL */
      }

    return list_copy;
} /* HDGLall_such_that() */

/******************************************************************************
 NAME
     HDGLremove_all_such_that
 DESCRIPTION
    This function will remove all objects in the list which cause the
    specified function to return a TRUE (non-zero) value.  Any optional
    arguments required can be passed through args.  Note that the memory
    for these objects will not be reclaimed, so if the objects have
    no other references, it is best to avoid this function and remove the
    objects one by one, freeing them when necessary.
 RETURNS
     Nothing
*******************************************************************************/
void
HDGLremove_all_such_that(Generic_list list, 
                     intn (*fn)(VOIDP /* pointer */, VOIDP /* args */), 
                     VOIDP args)
{
    VOIDP obj;

    /* reset to the beginning */
    HDGLreset_to_beginning(list);

    while ((obj = HDGLnext_in_list(list)))
      {
        if ((*fn)(obj, args))
            HDGLremove_current(list);
      }
} /* HDGLremove_HDGLall_such_that() */


#if 0
/****************************************************************************/
/****************************************************************************/
/**                                                                        **/
/**                         Internal functions                             **/
/**                                                                        **/
/****************************************************************************/
/****************************************************************************/

static void *
emalloc(unsigned int n)
{
    void *ptr;

    ptr = (void *) malloc(n);
    if ( ptr == NULL ) 
      {
        fprintf(stderr,"%s: error allocating memory\n", module);
        exit(1);
      }
    return ptr;
}
#endif
