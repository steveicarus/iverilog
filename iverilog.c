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
#ident "$Id: iverilog.c,v 1.14 2000/05/14 19:41:52 steve Exp $"
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

char warning_flags[16] = "";

char*inc_list = 0;
char*def_list = 0;

char*f_list = 0;

int synth_flag = 0;
int verbose_flag = 0;

char tmp[4096];

static int t_null(char*cmd, unsigned ncmd)
{
      int rc;

      sprintf(tmp, " | %s/ivl %s", base, warning_flags);
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

      sprintf(tmp, " | %s/ivl %s -o %s.cc -tvvm -Fcprop %s -Fnodangle"
	      " -fVPI_MODULE_PATH=%s", base, warning_flags, opath,
	      synth_flag?"-Fsynth -Fsyn-rules":"", base);

      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (f_list) {
	    rc = strlen(f_list);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, f_list);
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
	      "-L%s %s.cc -lvvm %s", CXX, opath, IVL_INC, IVL_LIB,
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

      sprintf(tmp, " | %s/ivl %s -o %s -txnf -Fcprop -Fsynth -Fsyn-rules "
	      "-Fnodangle -Fxnfio", base, warning_flags, opath);

      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (f_list) {
	    rc = strlen(f_list);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, f_list);
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

static void process_warning_switch(const char*name)
{
      if (warning_flags[0] == 0)
	    strcpy(warning_flags, "-W");

      if (strcmp(name,"all") == 0) {
	    strcat(warning_flags, "i");

      } else if (strcmp(name,"implicit") == 0) {
	    if (! strchr(warning_flags+2, 'i'))
		  strcat(warning_flags, "i");
      }
}

int main(int argc, char **argv)
{
      char*cmd;
      unsigned ncmd;
      int e_flag = 0;
      int opt, idx;
      char*cp;

      while ((opt = getopt(argc, argv, "B:D:Ef:I:o:Ss:t:vW:")) != EOF) {

	    switch (opt) {
		case 'B':
		  base = optarg;
		  break;
		case 'D':
		  if (def_list == 0) {
			def_list = malloc(strlen(" -D")+strlen(optarg)+1);
			strcpy(def_list, " -D");
			strcat(def_list, optarg);
		  } else {
			def_list = realloc(def_list, strlen(def_list)
					   + strlen(" -D")
					   + strlen(optarg) + 1);
			strcat(def_list, " -D");
			strcat(def_list, optarg);
		  }
		  break;
		case 'E':
		  e_flag = 1;
		  break;
		case 'f':
		  if (f_list == 0) {
			f_list = malloc(strlen(" -f")+strlen(optarg)+1);
			strcpy(f_list, " -f");
			strcat(f_list, optarg);
		  } else {
			f_list = realloc(f_list, strlen(f_list) +
					 strlen(" -f") +
					 strlen(optarg) + 1);
			strcat(f_list, " -f");
			strcat(f_list, optarg);
		  }
		  break;
		case 'I':
		  if (inc_list == 0) {
			inc_list = malloc(strlen(" -I")+strlen(optarg)+1);
			strcpy(inc_list, " -I");
			strcat(inc_list, optarg);
		  } else {
			inc_list = realloc(inc_list, strlen(inc_list)
					   + strlen(" -I")
					   + strlen(optarg) + 1);
			strcat(inc_list, " -I");
			strcat(inc_list, optarg);
		  }
		  break;
		case 'o':
		  opath = optarg;
		  break;
		case 'S':
		  synth_flag = 1;
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
		case 'W':
		  process_warning_switch(optarg);
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

      sprintf(tmp, "%s/ivlpp %s%s", base,
	      verbose_flag?" -v":"",
	      e_flag?"":" -L");

      ncmd = strlen(tmp);
      cmd = malloc(ncmd + 1);
      strcpy(cmd, tmp);

      if (inc_list) {
	    cmd = realloc(cmd, ncmd + strlen(inc_list) + 1);
	    strcat(cmd, inc_list);
	    ncmd += strlen(inc_list);
      }

      if (def_list) {
	    cmd = realloc(cmd, ncmd + strlen(def_list) + 1);
	    strcat(cmd, def_list);
	    ncmd += strlen(def_list);
      }

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
 * Revision 1.14  2000/05/14 19:41:52  steve
 *  Fix -f flag handling.
 *
 * Revision 1.13  2000/05/13 20:55:47  steve
 *  Use yacc based synthesizer.
 *
 * Revision 1.12  2000/05/09 00:02:13  steve
 *  Parameterize LD lib in C++ command line.
 *
 * Revision 1.11  2000/05/05 01:07:42  steve
 *  Add the -I and -D switches to iverilog.
 *
 * Revision 1.10  2000/05/04 20:08:20  steve
 *  Tell ivlpp to generate line number directives.
 *
 * Revision 1.9  2000/05/03 22:14:31  steve
 *  More features of ivl available through iverilog.
 *
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

