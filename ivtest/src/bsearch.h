/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef VCD_BSEARCH_NODES_VECTORS_H
#define VCD_BSEARCH_NODES_VECTORS_H

#include "globals.h"

hptr bsearch_node(nptr n, TimeType key);
vptr bsearch_vector(bvptr b, TimeType key);
char *bsearch_trunc(char *ascii, int maxlen);
struct symbol *bsearch_facs(struct globals *obj, char *ascii);

#endif
