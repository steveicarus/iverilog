/* 
 * Copyright (c) Tony Bybell 1999.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WAVE_ALLOCA_H
#define WAVE_ALLOCA_H
#include <stdlib.h>
#if HAVE_ALLOCA_H
#include <alloca.h>
#elif defined(__GNUC__)
#ifndef __MINGW32__
#ifndef alloca
#define alloca __builtin_alloca
#endif
#endif
#elif defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#endif
#define wave_alloca alloca
#endif 

/*
 * $Id: wavealloca.h,v 1.2 2007/08/26 21:35:50 gtkwave Exp $
 * $Log: wavealloca.h,v $
 * Revision 1.2  2007/08/26 21:35:50  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.1  2007/08/06 03:50:50  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1  2007/05/30 04:27:29  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:18  gtkwave
 * initial release
 *
 */

