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
#ident "$Id: iverilog.c,v 1.8 2000/05/01 23:55:22 steve Exp $"
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

char*f_list = 0;

int verbose_flag = 0;

char tmp[4096];

static int t_null(char*cmd, unsigned ncmd)
{
      int rc;

      sprintf(tmp, " | %s/ivl ", base);
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (start) {
	    sprintf(tmp, " -s%s", start);
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      if (verbose_flag) {
	    sprintf(tmp, " -v");
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      sprintf(tmp, " -- -");
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (verbose_flag)
	    printf("translate: %s\n", cmd);

      rc = system(cmd);
      return rc;
}

/*
 * This function handles the vvm target. After preprocessing, run the
 * ivl translator to get C++, then run g++ to make an executable
 * program out of that.
 */
static int t_vvm(char*cmd, unsigned ncmd)
{
      int rc;

      sprintf(tmp, " | %s/ivl -o %s.cc -tvvm -Fcprop -Fnodangle -fVPI_MODULE_PATH=%s", base, opath, base);

      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (f_list) {
	    rc = strlen(f_list);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      if (start) {
	    sprintf(tmp, " -s%s", start);
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }
      sprintf(tmp, " -- -");
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (verbose_flag)
	    printf("translate: %s\n", cmd);

      rc = system(cmd);
      if (rc != 0) {
	    fprintf(stderr, "errors translating Verilog program.\n");
	    return rc;
      }

      sprintf(tmp, "%s -O -rdynamic -fno-exceptions -o %s -I%s "
	      "-L%s %s.cc -lvvm -ldl", CXX, opath, IVL_INC, IVL_LIB,
	      opath, DLLIB);

      if (verbose_flag)
	    printf("compile: %s\n", tmp);

      rc = system(tmp);
      if (rc != 0) {
	    fprintf(stderr, "errors compiling translated program.\n");
	    return rc;
      }

      sprintf(tmp, "%s.cc", opath);
      unlink(tmp);

      return 0;
}

static int t_xnf(char*cmd, unsigned ncmd)
{
      int rc;

      sprintf(tmp, " | %s/ivl -o %s -txnf -Fcprop -Fsynth -Fnodangle -Fxnfio", base, opath);

      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (f_list) {
	    rc = strlen(f_list);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      if (start) {
	    sprintf(tmp, " -s%s", start);
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }
      sprintf(tmp, " -- -");
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (verbose_flag)
	    printf("translate: %s\n", cmd);

      rc = system(cmd);

      return rc;
}

int main(int argc, char **argv)
{
      char*cmd;
      unsigned ncmd;
      int e_flag = 0;
      int opt, idx;
      char*cp;

      while ((opt = getopt(argc, argv, "B:Ef:o:s:t:v")) != EOF) {

	    switch (opt) {
		case 'B':
		  base = optarg;
		  break;
		case 'E':
		  e_flag = 1;
		  break;
		case 'f':
		  if (f_list == 0) {
			f_list = malloc(strlen("-f")+strlen(optarg)+1);
			strcpy(f_list, "-f");
			strcat(f_list, optarg);
		  } else {
			f_list = realloc(f_list, strlen(f_list) +
					 strlen(" -f") +
					 strlen(optarg) + 1);
			strcat(f_list, " -f");
			strcat(f_list, optarg);
		  }
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

      if (optind == argc) {
	    fprintf(stderr, "%s: No input files.\n", argv[0]);
	    return 1;
      }

	/* Start building the preprocess command line. */

      sprintf(tmp, "%s/ivlpp", base);
      if (verbose_flag)
	    strcat(tmp, " -v");

      ncmd = strlen(tmp);
      cmd = malloc(ncmd + 1);
      strcpy(cmd, tmp);

	/* Add all the verilog source files to the preprocess command line. */

      for (idx = optind ;  idx < argc ;  idx += 1) {
	    sprintf(tmp, " %s", argv[idx]);
	    cmd = realloc(cmd, ncmd+strlen(tmp)+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += strlen(tmp);
      }


	/* If the -E flag was given on the command line, then all we
	   do is run the preprocessor and put the output where the
	   user wants it. */
      if (e_flag) {
	    if (strcmp(opath,"-") != 0) {
		  sprintf(tmp, " > %s", opath);
		  cmd = realloc(cmd, ncmd+strlen(tmp)+1);
		  strcpy(cmd+ncmd, tmp);
		  ncmd += strlen(tmp);
	    }

	    if (verbose_flag)
		  printf("preprocess: %s\n", cmd);

	    return system(cmd);
      }

      if (strcmp(targ,"null") == 0)
	    return t_null(cmd, ncmd);
      else if (strcmp(targ,"vvm") == 0)
	    return t_vvm(cmd, ncmd);
      else if (strcmp(targ,"xnf") == 0)
	    return t_xnf(cmd, ncmd);
      else {
	    fprintf(stderr, "Unknown target: %s\n", targ);
	    return 1;
      }

      return 0;
}

/*
 * $Log: iverilog.c,v $
 * Revision 1.8  2000/05/01 23:55:22  steve
 *  Better inc and lib paths for iverilog.
 *
 * Revision 1.7  2000/04/29 01:20:14  steve
 *  The -f flag is now in place.
 *
 * Revision 1.6  2000/04/26 21:11:41  steve
 *  Mussed up command string mashing.
 *
 * Revision 1.5  2000/04/26 03:33:32  steve
 *  Do not set width too small to hold significant bits.
 *
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

