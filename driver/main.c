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
#ident "$Id: main.c,v 1.10 2001/05/17 03:14:26 steve Exp $"
#endif

const char HELP[] =
"Usage: iverilog [-ESv] [-B base] [-C path] [-c cmdfile]
                [-D macro[=defn]] [-I includedir] [-m module]
                [-N file] [-o filename] [-p flag=value]
                [-s topmodule] [-t target] [-T min|typ|max]
                [-W class] source_file(s)
See man page for details.";

#define MAXSIZE 4096

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#ifndef IVL_ROOT
# define IVL_ROOT "."
#endif

#ifndef RDYNAMIC
# define RDYNAMIC "-rdynamic"
#endif

# include  "globals.h"

const char*base = IVL_ROOT;
const char*mtm  = 0;
const char*opath = "a.out";
const char*npath = 0;
const char*targ  = "vvm";
const char*start = 0;

char warning_flags[16] = "";

char*inc_list = 0;
char*def_list = 0;
char*mod_list = 0;
char*command_filename = 0;

char*f_list = 0;

int synth_flag = 0;
int verbose_flag = 0;
int command_file = 0;
int inside_c_comment = 0;

FILE *fp;

char line[MAXSIZE];
char tmp[MAXSIZE];

/*
 * This is the default target type. It looks up the bits that are
 * needed to run the command from the configuration file (which is
 * already parsed for us) so we can handle must of the generic cases.
 */
static int t_default(char*cmd, unsigned ncmd)
{
      int rc;
      const char*pattern;

      pattern = lookup_pattern("<ivl>");
      if (pattern == 0) {
	    fprintf(stderr, "No such target: %s\n", targ);
	    return -1;
      }

      tmp[0] = ' ';
      tmp[1] = '|';
      tmp[2] = ' ';
      rc = build_string(tmp+3, sizeof tmp - 3, pattern);
      cmd = realloc(cmd, ncmd+3+rc+1);
      strcpy(cmd+ncmd, tmp);


      if (verbose_flag)
	    printf("translate: %s\n", cmd);


      rc = system(cmd);
      if (rc != 0) {
	    if (WIFEXITED(rc))
		  return WEXITSTATUS(rc);

	    fprintf(stderr, "Command signaled: %s\n", cmd);
	    return -1;
      }

      return 0;
}

/*
 * This function handles the vvm target. After preprocessing, run the
 * ivl translator to get C++, then run g++ to make an executable
 * program out of that.
 */
static int t_vvm(char*cmd, unsigned ncmd)
{
      int rc;

      const char*pattern = lookup_pattern("<ivl>");
      if (pattern == 0) {
	    fprintf(stderr, "No such target: %s\n", targ);
	    return -1;
      }

      tmp[0] = ' ';
      tmp[1] = '|';
      tmp[2] = ' ';
      rc = build_string(tmp+3, sizeof tmp - 3, pattern);
      cmd = realloc(cmd, ncmd+3+rc+1);
      strcpy(cmd+ncmd, tmp);

      if (verbose_flag)
	    printf("translate: %s\n", cmd);

      rc = system(cmd);
      if (rc != 0) {
	    if (WIFEXITED(rc)) {
		  fprintf(stderr, "errors translating Verilog program.\n");
		  return WEXITSTATUS(rc);
	    } else {
		  fprintf(stderr, "Command signaled: %s\n", cmd);
		  return -1;
	    }
      }

      sprintf(tmp, "%s " RDYNAMIC " -fno-exceptions -o %s -I%s "
	      "-L%s %s.cc -lvvm -lvpip %s", CXX, opath, IVL_INC, IVL_LIB,
	      opath, DLLIB);

      if (verbose_flag)
	    printf("compile: %s\n", tmp);

      rc = system(tmp);
      if (rc != 0) {
	    if (WIFEXITED(rc)) {
		  fprintf(stderr, "errors compiling translated program.\n");
		  return WEXITSTATUS(rc);
	    } else {
		  fprintf(stderr, "Command signaled: %s\n", tmp);
		  return -1;
	    }
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

      if (mtm) {
	    sprintf(tmp, " -T%s", mtm);
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      if (npath) {
	    sprintf(tmp, " -N%s", npath);
	    rc = strlen(tmp);
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
	    if (WIFEXITED(rc)) {
		  fprintf(stderr, "errors translating Verilog program.\n");
		  return WEXITSTATUS(rc);
	    }

	    fprintf(stderr, "Command signaled: %s\n", cmd);
	    return -1;
      }

      return 0;
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
      const char*config_path = 0;
      char*cmd;
      unsigned ncmd;
      int e_flag = 0;
      int opt, idx;
      char*cp;

      while ((opt = getopt(argc, argv, "B:C:c:D:Ef:hI:m:N::o:p:Ss:T:t:vW:")) != EOF) {

	    switch (opt) {
		case 'B':
		  base = optarg;
		  break;
		case 'C':
		  config_path = optarg;
		  break;
 		case 'c':
		  command_filename = malloc(strlen(optarg)+1);
 		  strcat(command_filename, optarg);
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
		  fprintf(stderr, "warning: The -f flag is moved to -p\n");
		case 'p':
		  if (f_list == 0) {
			f_list = malloc(strlen(" -p")+strlen(optarg)+1);
			strcpy(f_list, " -p");
			strcat(f_list, optarg);
		  } else {
			f_list = realloc(f_list, strlen(f_list) +
					 strlen(" -p") +
					 strlen(optarg) + 1);
			strcat(f_list, " -p");
			strcat(f_list, optarg);
		  }
		  break;

 		case 'h':
 		  fprintf(stderr, "%s\n", HELP);
 		  return 1;

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
		case 'm':
		  if (mod_list == 0) {
			mod_list = malloc(strlen(" -m")+strlen(optarg)+1);
			strcpy(mod_list, " -m");
			strcat(mod_list, optarg);
		  } else {
			mod_list = realloc(mod_list, strlen(mod_list)
					   + strlen(" -m")
					   + strlen(optarg) + 1);
			strcat(mod_list, " -m");
			strcat(mod_list, optarg);
		  }
		  break;

		case 'N':
		  npath = optarg;
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
		case 'T':
		  if (strcmp(optarg,"min") == 0) {
			mtm = "min";
		  } else if (strcmp(optarg,"typ") == 0) {
			mtm = "typ";
		  } else if (strcmp(optarg,"max") == 0) {
			mtm = "max";
		  } else {
			fprintf(stderr, "%s: invalid -T%s argument\n",
				argv[0], optarg);
			return 1;
		  }
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

      if ((optind == argc) && !command_filename) {
	    fprintf(stderr, "%s: No input files.\n", argv[0]);
 	    fprintf(stderr, "%s\n", HELP);
	    return 1;
      }

	/* Load the iverilog.conf file to get our substitution
	   strings. */

      { char path[1024];
        FILE*fd;
	if (config_path) {
	      strcpy(path, config_path);
	} else {
	      sprintf(path, "%s/iverilog.conf", base);
	}
	fd = fopen(path, "r");
	if (fd == 0) {
	      fprintf(stderr, "Config file \"%s\" not found\n",path);
	      return 1;
	}
	reset_lexor(fd);
	yyparse();
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

	/* If user supplied a command file, retain its contents -- this
           process supersedes command line source files. */

      if (command_filename != 0) {
	    if (( fp = fopen(command_filename, "r")) == NULL ) {
		  fprintf(stderr, "%s: Can't open %s\n",
			  argv[0], command_filename);
		  return 1;

	    } else {
		    /* Process file and skip over commented-out lines.
		       Skips over c-like comment as well as '//' or
		       '#' lines. */

		  sprintf(tmp, "");
		  while (fgets(line, MAXSIZE, fp) != NULL) {
			if ( strstr(line, "*/") != NULL ) {
			      inside_c_comment = 0;
			      continue;
			}

			if (inside_c_comment || (strstr(line, "/*") != NULL)) {
			      inside_c_comment = 1;
			      continue;
			}

			if ( (line[0] != '/' || line[1] != '/')
			     && line[0] != '#' ) {
			      strcat  (tmp, " ");
			      strncat (tmp, line, (strlen(line)-1));
			}
		  }
		  fclose(fp);
	    }
	    cmd = realloc(cmd, ncmd+strlen(tmp)+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += strlen(tmp);

      } else {

	      /* Add all verilog source files to the preprocess
		 command line. */

	    for (idx = optind ;  idx < argc ;  idx += 1) {
		  sprintf(tmp, " %s", argv[idx]);
		  cmd = realloc(cmd, ncmd+strlen(tmp)+1);
		  strcpy(cmd+ncmd, tmp);
		  ncmd += strlen(tmp);
	    }
      }

	/* If the -E flag was given on the command line, then all we
	   do is run the preprocessor and put the output where the
	   user wants it. */
      if (e_flag) {
	    int rc;
	    if (strcmp(opath,"-") != 0) {
		  sprintf(tmp, " > %s", opath);
		  cmd = realloc(cmd, ncmd+strlen(tmp)+1);
		  strcpy(cmd+ncmd, tmp);
		  ncmd += strlen(tmp);
	    }

	    if (verbose_flag)
		  printf("preprocess: %s\n", cmd);

	    rc = system(cmd);
	    if (rc != 0) {
		  if (WIFEXITED(rc)) {
			fprintf(stderr, "errors preprocessing Verilog program.\n");
			return WEXITSTATUS(rc);
		  }

		  fprintf(stderr, "Command signaled: %s\n", cmd);
		  return -1;
	    }

	    return 0;
      }

      if (strcmp(targ,"vvm") == 0)
	    return t_vvm(cmd, ncmd);
      else if (strcmp(targ,"xnf") == 0)
	    return t_xnf(cmd, ncmd);
      else {
	    return t_default(cmd, ncmd);
      }

      return 0;
}

/*
 * $Log: main.c,v $
 * Revision 1.10  2001/05/17 03:14:26  steve
 *  Update help message.
 *
 * Revision 1.9  2001/04/26 16:04:39  steve
 *  Handle missing or uninstalled .conf files.
 *
 * Revision 1.8  2001/02/01 17:12:22  steve
 *  Forgot to actually allow the -p flag.
 *
 * Revision 1.7  2001/01/20 19:02:05  steve
 *  Switch hte -f flag to the -p flag.
 *
 * Revision 1.6  2001/01/20 02:15:51  steve
 *  apologize for not supporting non-constant delays.
 *
 * Revision 1.5  2000/11/30 02:50:54  steve
 *  Add command file (-c) support from Nadim Shaikli.
 *
 * Revision 1.4  2000/10/28 17:28:16  steve
 *  Split vpip for everybody.
 *
 * Revision 1.3  2000/10/28 03:47:25  steve
 *  Use the conf file to generate the vvm ivl string.
 *
 * Revision 1.2  2000/10/28 03:45:47  steve
 *  Use the conf file to generate the vvm ivl string.
 *
 * Revision 1.1  2000/10/08 22:36:56  steve
 *  iverilog with an iverilog.conf configuration file.
 *
 */

