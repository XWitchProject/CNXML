/*
 * Generic hashmap manipulation functions
 *
 * Originally by Elliot C Back - http://elliottback.com/wp/hashmap-implementation-in-c/
 *
 * Modified by Pete Warden to fix a serious performance problem, support strings as keys
 * and removed thread synchronization - http://petewarden.typepad.com
 */
#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "cnxml_string.h"
#include "cnxml_common.h"
#include <stddef.h>

#define CNXML_MAP_MISSING -3  /* No such element */
#define CNXML_MAP_FULL -2   /* Hashmap is full */
#define CNXML_MAP_OMEM -1   /* Out of Memory */
#define CNXML_MAP_OK 0  /* OK */

/*
 * cnxml_any is a pointer.  This allows you to put arbitrary structures in
 * the hashmap.
 */
typedef void *cnxml_any;

/*
 * cnxml_hashmap_iter_func is a pointer to a function that can take two
 * cnxml_any arguments + a cnxml_string and return an integer. Returns
 * status code..
 */
typedef int (*cnxml_hashmap_iter_func)(cnxml_any, cnxml_string, cnxml_any);

/*
 * cnxml_map is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented.  They see and manipulate only cnxml_map's.
 */
typedef cnxml_any cnxml_map;

/*
 * Return an empty hashmap. Returns NULL if empty.
*/
CNXML_EXPORT extern cnxml_map cnxml_hashmap_new(cnxml_context* ctx);

/*
 * Iteratively call f with argument (item, data) for
 * each element data in the hashmap. The function must
 * return a map status code. If it returns anything other
 * than CNXML_MAP_OK the traversal is terminated. f must
 * not reenter any hashmap functions, or deadlock may arise.
 */
CNXML_EXPORT extern int cnxml_hashmap_iterate(cnxml_map in, cnxml_hashmap_iter_func f, cnxml_any item);

/*
 * Add an element to the hashmap. Return CNXML_MAP_OK or CNXML_MAP_OMEM.
 */
CNXML_EXPORT extern int cnxml_hashmap_put(cnxml_map in, cnxml_string key, cnxml_any value);

/*
 * Get an element from the hashmap. Return CNXML_MAP_OK or CNXML_MAP_MISSING.
 */
CNXML_EXPORT extern int cnxml_hashmap_get(cnxml_map in, cnxml_string key, cnxml_any *arg);

/*
 * Remove an element from the hashmap. Return CNXML_MAP_OK or CNXML_MAP_MISSING.
 */
CNXML_EXPORT extern int cnxml_hashmap_remove(cnxml_map in, cnxml_string key);

/*
 * Get any element. Return CNXML_MAP_OK or CNXML_MAP_MISSING.
 * remove - should the element be removed from the hashmap
 */
CNXML_EXPORT extern int cnxml_hashmap_get_one(cnxml_map in, cnxml_any *arg, int remove);

/*
 * Free the hashmap
 */
CNXML_EXPORT extern void cnxml_hashmap_free(cnxml_map in);

/*
 * Get the current size of a hashmap
 */
CNXML_EXPORT extern int cnxml_hashmap_length(cnxml_map in);

#endif __HASHMAP_H__
