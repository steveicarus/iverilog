/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: iverilog.c,v 1.4 2000/04/23 21:14:32 steve Exp $"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef IVL_ROOT
# define IVL_ROOT "."
#endif

const char*base = IVL_ROOT;
const char*opath = "a.out";
const char*targ  = "vvm";
const char*start = 0;

int verbose_flag = 0;

static char cmdline[8192];

static int t_null()
{
      int rc;

      strcat(cmdline, " | ");
      strcat(cmdline, base);
      strcat(cmdline, "/ivl ");
      if (start) {
	    strcat(cmdline, " -s ");
	    strcat(cmdline, start);
      }
      if (verbose_flag)
	    strcat(cmdline, "-v ");
      strcat(cmdline, "-- -");

      if (verbose_flag)
	    printf("translate: %s\n", cmdline);

      rc = system(cmdline);
      return rc;
}

/*
 * This function handles the vvm target. After preprocessing, run the
 * ivl translator to get C++, then run g++ to make an executable
 * program out of that.
 */
static int t_vvm()
{
      int rc;

      strcat(cmdline, " | ");
      strcat(cmdline, base);
      strcat(cmdline, "/ivl -o ");
      strcat(cmdline, opath);
      strcat(cmdline, ".cc -tvvm -Fcprop -Fnodangle -fVPI_MODULE_PATH=");
      strcat(cmdline, base);
      if (start) {
	    strcat(cmdline, " -s ");
	    strcat(cmdline, start);
      }
      strcat(cmdline, " -- -");
      if (verbose_flag)
	    printf("translate: %s\n", cmdline);
      rc = system(cmdline);

      if (rc != 0) {
	    fprintf(stderr, "errors translating Verilog program.\n");
	    return rc;
      }

      sprintf(cmdline, "g++ -O -rdynamic -fno-exceptions -o %s -I%s "
	      "-L%s %s.cc -lvvm -ldl", opath, base, base, opath);

      if (verbose_flag)
	    printf("compile: %s\n", cmdline);

      rc = system(cmdline);
      if (rc != 0) {
	    fprintf(stderr, "errors compiling translated program.\n");
	    return rc;
      }

      sprintf(cmdline, "%s.cc", opath);
      unlink(cmdline);

      return 0;
}

static int t_xnf()
{
      int rc;

      strcat(cmdline, " | ");
      strcat(cmdline, base);
      strcat(cmdline, "/ivl -o ");
      strcat(cmdline, opath);
      if (start) {
	    strcat(cmdline, " -s ");
	    strcat(cmdline, start);
      }
      strcat(cmdline, " -txnf -Fcprop -Fsynth -Fnodangle -Fxnfio");
      strcat(cmdline, " -- -");
      if (verbose_flag)
	    printf("translate: %s\n", cmdline);
      rc = system(cmdline);

      return rc;
}

int main(int argc, char **argv)
{
      int e_flag = 0;
      int opt, idx;
      char*cp;

      while ((opt = getopt(argc, argv, "B:Eo:s:t:v")) != EOF) {

	    switch (opt) {
		case 'B':
		  base = optarg;
		  break;
		case 'E':
		  e_flag = 1;
		  break;
		case 'o':
		  opath = optarg;
		  break;
		case 's':
		  start = optarg;
		  break;
		case 't':
		  targ = optarg;
		  break;
		case 'v':
		  verbose_flag = 1;
		  break;
		case '?':
		default:
		  return 1;
	    }
      }

	/* Now collect the verilog source files. */

      strcpy(cmdline, base);
      cp = cmdline + strlen(cmdline);
      strcpy(cp, "/ivlpp");
      cp += strlen(cp);
      if (verbose_flag) {
	    strcpy(cp, " -v");
	    cp += strlen(cp);
      }

      if (optind == argc) {
	    fprintf(stderr, "%s: No input files.\n", argv[0]);
	    return 1;
      }

      for (idx = optind ;  idx < argc ;  idx += 1) {
	    sprintf(cp, " %s", argv[idx]);
	    cp += strlen(cp);
      }

	/* If the -E flag was given on the command line, then all we
	   do is run the preprocessor and put the output where the
	   user wants it. */
      if (e_flag) {
	    if (strcmp(opath,"-") != 0) {
		  sprintf(cp, " > %s", opath);
		  cp += strlen(cp);
	    }
	    if (verbose_flag)
		  printf("preprocess: %s\n", cmdline);

	    return system(cmdline);
      }

      if (strcmp(targ,"null") == 0)
	    return t_null();
      else if (strcmp(targ,"vvm") == 0)
	    return t_vvm();
      else if (strcmp(targ,"xnf") == 0)
	    return t_xnf();
      else {
	    fprintf(stderr, "Unknown target: %s\n", targ);
	    return 1;
      }

      return 0;
}

/*
 * $Log: iverilog.c,v $
 * Revision 1.4  2000/04/23 21:14:32  steve
 *  The -s flag.
 *
 * Revision 1.3  2000/04/21 22:54:47  steve
 *  module path in vvm target.
 *
 * Revision 1.2  2000/04/21 22:51:38  steve
 *  Support the -tnull target type.
 *
 * Revision 1.1  2000/04/21 06:41:03  steve
 *  Add the iverilog driver program.
 *
 */

