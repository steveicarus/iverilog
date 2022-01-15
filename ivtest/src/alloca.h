/*
 * Copyright (c) Tony Bybell 1999-2000.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef VCD_ALLOCA_H
#define VCD_ALLOCA_H
#include <stdlib.h>

/*
 * if your system really doesn't have alloca() at all,
 * you can force functionality by using malloc
 * instead.  but note that you're going to have some
 * memory leaks because of it.  you have been warned.
 */

#ifdef _MSC_VER
	#define alloca _alloca
#endif

#ifndef __sun__
	#ifndef alloca
	#define alloca __alloca
	#endif
#else
	#include <alloca.h>
#endif

#define wave_alloca alloca

#endif
