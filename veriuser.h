#ifndef __veriuser_H
#define __veriuser_H
/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: veriuser.h,v 1.9 2002/05/31 18:25:51 steve Exp $"
#endif

/*
 * This header file contains the definitions and declarations needed
 * by an Icarus Verilog user using tf_ routines.
 *
 * NOTE: Icarus Verilog does not support tf_ routines. This is just a
 * stub. The functions that are implemented here are actually
 * implemented using VPI routines.
 */

#ifdef __cplusplus
# define EXTERN_C_START extern "C" {
# define EXTERN_C_END }
#else
# define EXTERN_C_START
# define EXTERN_C_END
#endif

#ifndef __GNUC__
# undef  __attribute__
# define __attribute__(x)
#endif

EXTERN_C_START

/*
 * structure for veriusertfs array
 */
typedef struct t_tfcell
{
      short type;               /* usertask|userfunction|userrealfunction */
      short data;               /* data passed to user routine */
      int   (*checktf)();       /* pointer to checktf routine */
      int   (*sizetf)();        /* pointer to sizetf routine */
      int   (*calltf)();        /* pointer to calltf routine */
      int   (*misctf)();        /* pointer to misctf routine */
      char  *tfname;            /* name of the system task/function */
      int   forwref;            /* usually set to 1 */
      char  *tfveritool;        /* usually ignored */
      char  *tferrmessage;      /* usually ignored */
      char  reserved[20];            /* reserved */
} s_tfcell, *p_tfcell;

extern s_tfcell veriusertfs[];
extern void veriusertfs_register();

#define usertask 1
#define userfunction 2
#define userrealfunction 3

/* misctf callback reasons */
#define reason_paramvc 7
#define reason_finish 9
#define reason_endofcompile 16


/* Extern functions from the library. */
extern void io_printf (const char *, ...)
      __attribute__((format (printf,1,2)));
extern char* mc_scan_plusargs(char*plusarg);

extern int tf_dofinish(void);

extern int tf_dostop(void);

extern void tf_error(const char*, ...)
      __attribute__((format (printf,1,2)));

extern int tf_getlongtime(int*high_bits);

extern int tf_nump(void);

extern void tf_warning(const char*, ...)
      __attribute__((format (printf,1,2)));

EXTERN_C_END

/*
 * $Log: veriuser.h,v $
 * Revision 1.9  2002/05/31 18:25:51  steve
 *  Add tf_getlongtime (mruff)
 *
 * Revision 1.8  2002/05/31 04:26:44  steve
 *  Call padding reserved.
 *
 * Revision 1.7  2002/05/30 02:37:26  steve
 *  Add the veriusertf_register funciton.
 *
 * Revision 1.6  2002/05/30 02:12:17  steve
 *  Add tf_nump from mruff.
 *
 * Revision 1.5  2002/05/30 02:10:08  steve
 *  Add tf_error and tf_warning from mruff
 *
 * Revision 1.4  2002/05/24 20:29:07  steve
 *  Implement mc_scan_plusargs.
 *
 * Revision 1.3  2002/05/24 19:05:30  steve
 *  support GCC __attributes__ for printf formats.
 *
 * Revision 1.2  2002/05/23 03:35:42  steve
 *  Add the io_printf function to libveriuser.
 *
 * Revision 1.1  2002/05/19 05:21:00  steve
 *  Start the libveriuser library.
 *
 */
#endif
