/*
 * Copyright (c) 2000-2009 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: main.c,v 1.65.2.6 2007/05/30 17:48:26 steve Exp $"
#endif

# include "config.h"
# include "version.h"

const char NOTICE[] =
"  This program is free software; you can redistribute it and/or modify\n"
"  it under the terms of the GNU General Public License as published by\n"
"  the Free Software Foundation; either version 2 of the License, or\n"
"  (at your option) any later version.\n"
"\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"  GNU General Public License for more details.\n"
"\n"
"  You should have received a copy of the GNU General Public License\n"
"  along with this program; if not, write to the Free Software\n"
"  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA\n"
;

const char HELP[] =
"Usage: iverilog [-EhSvV] [-B base] [-c cmdfile] [-g1|-g2|-g3.0]\n"
"                [-D macro[=defn]] [-I includedir] [-M depfile] [-m module]\n"
"                [-N file] [-o filename] [-p flag=value]\n"
"                [-s topmodule] [-t target] [-T min|typ|max]\n"
"                [-W class] [-y dir] [-Y suf] source_file(s)\n"
"See man page for details.";

#define MAXSIZE 4096

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef __MINGW32__
#include <windows.h>
#ifdef HAVE_LIBIBERTY_H
#include <libiberty.h>
#endif
#endif

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern int getopt(int argc, char*argv[], const char*fmt);
extern int optind;
extern const char*optarg;
#endif

#if !defined(WIFEXITED)
# define WIFEXITED(rc) ((rc&0x7f) == 0)
#endif

#if !defined(WEXITSTATUS)
# define WEXITSTATUS(rc) (rc>>8)
#endif

#ifndef IVL_ROOT
# define IVL_ROOT "."
#endif

#ifndef IVL_ROOT_VARIABLE
# define IVL_ROOT_VARIABLE "IVERILOG_ROOT"
#endif

# include  "globals.h"
#include "cfparse_misc.h"   /* cfparse() */

#ifdef __MINGW32__
const char sep = '\\';
#else
const char sep = '/';
#endif

extern void cfreset(FILE*fd, const char*path);

const char*base = 0;
const char*pbase = 0;
const char*mtm  = 0;
const char*opath = "a.out";
const char*npath = 0;
const char*targ  = "vvp";
const char*depfile = 0;

const char*generation = "3.0";

char warning_flags[16] = "";

char*inc_list = 0;
char*def_list = 0;
char*mod_list = 0;
char*command_filename = 0;

/* These are used to collect the list of file names that will be
   passed to ivlpp. Keep the list in a file because it can be a long
   list. */
char*source_path = 0;
FILE*source_file = 0;
unsigned source_count = 0;

char*iconfig_path = 0;
FILE*iconfig_file = 0;

static char iconfig_common_path_buf[4096] = "";
char*iconfig_common_path = iconfig_common_path_buf;

int synth_flag = 0;
int verbose_flag = 0;
int command_file = 0;

FILE *fp;

char line[MAXSIZE];
char tmp[MAXSIZE];

static char ivl_root[MAXSIZE];

#ifdef __MINGW32__
# include  <io.h>
# include  <fcntl.h>
static FILE*fopen_safe(const char*path)
{
      FILE*file = 0;
      int fd;

      fd = _open(path, _O_WRONLY|_O_CREAT|_O_EXCL, 0700);
      if (fd != -1)
	    file = _fdopen(fd, "w");

       return file;
}
#else
# include  <fcntl.h>
static FILE*fopen_safe(const char*path)
{
      FILE*file = 0;
      int fd;

      fd = open(path, O_WRONLY|O_CREAT|O_EXCL, 0700);
      if (fd != -1)
	    file = fdopen(fd, "w");

       return file;
}
#endif

static const char*my_tempfile(const char*str, FILE**fout)
{
      FILE*file;
      int retry;

      static char pathbuf[8192];

      const char*tmpdir = getenv("TMP");
      if (tmpdir == 0)
	    tmpdir = getenv("TMPDIR");
      if (tmpdir == 0)
	    tmpdir = getenv("TEMP");
#ifdef __MINGW32__
      if (tmpdir == 0)
	    tmpdir = "C:\\TEMP";
#else
      if (tmpdir == 0)
	    tmpdir = "/tmp";
#endif

      assert(tmpdir);
      assert((strlen(tmpdir) + strlen(str)) < sizeof pathbuf - 10);

      srand(getpid());
      retry = 100;
      file = NULL;
      while ((retry > 0) && (file == NULL)) {
	    unsigned code = rand();
	    sprintf(pathbuf, "%s%c%s%04x", tmpdir, sep, str, code);
	    file = fopen_safe(pathbuf);
	    retry -= 1;
      }

      *fout = file;
      return pathbuf;
}

/*
 * This is the default target type. It looks up the bits that are
 * needed to run the command from the configuration file (which is
 * already parsed for us) so we can handle must of the generic cases.
 */
static int t_default(char*cmd, unsigned ncmd)
{
      unsigned rc;
#ifdef __MINGW32__
      unsigned ncmd_start = ncmd;
#endif

      snprintf(tmp, sizeof tmp, " | %s/ivl", base);
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (verbose_flag) {
	    const char*vv = " -v";
	    rc = strlen(vv);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, vv);
	    ncmd += rc;
      }

      if (npath != 0) {
	    snprintf(tmp, sizeof tmp, " -N%s", npath);
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      snprintf(tmp, sizeof tmp, " -C%s", iconfig_path);
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      snprintf(tmp, sizeof tmp, " -C%s -- -", iconfig_common_path);
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

#ifdef __MINGW32__
      {
	char *t;
	for (t = cmd+ncmd_start; *t; t++)
	  {
	    if (*t == '/') *t = '\\';
	  }
      }
#endif

#ifdef DEBUG
      char *original_cmd = malloc( strlen( cmd ) + (size_t)1 );
      strcpy( original_cmd, cmd );

      cmd = realloc( cmd, strlen( original_cmd ) + (size_t)19 ); /* more than enough memory for the ivlpp command and ">/tmp/ivlpp_stdout" */
      if( cmd == NULL ){
	exit( EXIT_FAILURE );
      }

      char *debug_ivl = malloc( strlen( original_cmd ) + (size_t)5 ); /* more than enough memory for "gdb " and ivl program_name */
      strcpy( debug_ivl, "gdb " );

      static char whitespace[] = " \t\f\r\v\n";
      char *token;
      int found_pipe = 0;
      unsigned long token_counter = 0UL;

      strcpy( cmd, "" );
      for( token = strtok( original_cmd, whitespace ); token != NULL; token = strtok( NULL, whitespace ) ){

	if( strcmp( token, "|" ) == 0 ){
	  found_pipe = 1;
	}

	if( found_pipe ){
	  token_counter++;
	}

	if( token_counter == 0UL ){
	  strcat( cmd, token );
	  strcat( cmd, " " ); /* harmless space after last token, and it was already there anyway */
	}

	if( token_counter == 2UL ){
	  strcat( debug_ivl, token );
	}

	if( token_counter > 2UL ){
	  printf( "%s\n", token );
	}

      }

      free( original_cmd );
#ifdef __MINGW32__
      strcat( cmd, ">%TMP%\\ivlpp_stdout" );
#else
      strcat( cmd, ">/tmp/ivlpp_stdout" );
#endif
#endif

      if (verbose_flag) {
	    printf("translate: %s\n", cmd);
	    fflush(stdout);
      }

      rc = system(cmd);
#ifdef DEBUG
      if( verbose_flag ){
	printf( "%s\n", debug_ivl );
      }

      system( debug_ivl ); /* For simplicity, ignore any return value, which is implementation dependent anyway. */
      free( debug_ivl );
#endif
      remove(source_path);
      if ( ! getenv("IVERILOG_ICONFIG"))
	    remove(iconfig_path);
#ifdef __MINGW32__  /* MinGW just returns the exit status, so return it! */
      return rc;
#else
      if (rc != 0) {
	    if (rc == 127) {
		  fprintf(stderr, "Failed to execute: %s\n", cmd);
		  return 1;
	    }

	    if (WIFEXITED(rc))
		  return WEXITSTATUS(rc);

	    fprintf(stderr, "Command signaled: %s\n", cmd);
	    return -1;
      }

      return 0;
#endif
}


static void process_warning_switch(const char*name)
{
      if (strcmp(name,"all") == 0) {
	    strcat(warning_flags, "iptu");

      } else if (strcmp(name,"implicit") == 0) {
	    if (! strchr(warning_flags+2, 'i'))
		  strcat(warning_flags, "i");
      } else if (strcmp(name,"portbind") == 0) {
	    if (! strchr(warning_flags+2, 'p'))
		  strcat(warning_flags, "p");
      } else if (strcmp(name,"timescale") == 0) {
	    if (! strchr(warning_flags+2, 't'))
		  strcat(warning_flags, "t");
      } else if (strcmp(name,"unused") == 0) {
	    if (! strchr(warning_flags+2, 'u'))
		  strcat(warning_flags, "u");
      } else if (strcmp(name,"no-implicit") == 0) {
	    char*cp = strchr(warning_flags+2, 'i');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-portbind") == 0) {
	    char*cp = strchr(warning_flags+2, 'p');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-timescale") == 0) {
	    char*cp = strchr(warning_flags+2, 't');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-unused") == 0) {
	    char*cp = strchr(warning_flags+2, 'u');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      }
}

void process_library_switch(const char *name)
{
      fprintf(iconfig_file, "-y:%s\n", name);
}

void process_library_nocase_switch(const char *name)
{
      fprintf(iconfig_file, "-yl:%s\n", name);
}

void process_library2_switch(const char *name)
{
      fprintf(iconfig_file, "-Y:%s\n", name);
}

void process_include_dir(const char *name)
{
      if (inc_list == 0) {
	    inc_list = malloc(strlen(" -I")+strlen(name)+1);
	    strcpy(inc_list, " -I");
	    strcat(inc_list, name);
      } else {
	    inc_list = realloc(inc_list, strlen(inc_list)
			       + strlen(" -I")
			       + strlen(name) + 1);
	    strcat(inc_list, " -I");
	    strcat(inc_list, name);
      }
}

void process_define(const char*name)
{
      if (def_list == 0) {
	    def_list = malloc(strlen(" -D")+strlen(name)+1);
	    strcpy(def_list, " -D");
	    strcat(def_list, name);
      } else {
	    def_list = realloc(def_list, strlen(def_list)
			       + strlen(" -D")
			       + strlen(name) + 1);
	    strcat(def_list, " -D");
	    strcat(def_list, name);
      }
}

/*
 * This function is called while processing a file name in a command
 * file, or a file name on the command line. Look to see if there is a
 * .sft suffix, and if so pass that as a sys_func file. Otherwise, it
 * is a Verilog source file to be written into the file list.
 */
void process_file_name(const char*name)
{
      if (strlen(name) > 4 && strcasecmp(".sft", name+strlen(name)-4) == 0) {
	    fprintf(iconfig_file,"sys_func:%s\n", name);

      } else {
	    fprintf(source_file, "%s\n", name);
	    source_count += 1;
      }
}

int process_generation(const char*name)
{
      if (strcmp(name,"1") == 0)
	    generation = "1";

      else if (strcmp(name,"2") == 0)
	    generation = "2";

      else if (strcmp(name,"3.0") == 0)
	    generation = "3.0";

      else {
	    fprintf(stderr, "Unknown/Unsupported Language generation "
		    "%s\n", name);
	    fprintf(stderr, "Supported generations are:\n");
	    fprintf(stderr, "    1   -- IEEE1364-1995 (Verilog 1)\n"
		            "    2   -- IEEE1364-2001 (Verilog 2001)\n"
		            "    3.0 -- SystemVerilog 3.0\n");
	    return 1;
      }

      return 0;
}

/*
 * This function fills in th ivl_root directory with the appropriate
 * path, based on operating system, build time configuration, and
 * environment.
 */
static char* get_root_dir()
{
#ifdef __MINGW32__
      { char * s;
	char basepath[1024];
	GetModuleFileName(NULL,basepath,1024);

	  /* Calculate the ivl_root from the path to the command. This
	     is necessary because of the installation process in
	     Windows. Mostly, it is those darn drive letters, but oh
	     well. We know the command path is formed like this:

	         D:\iverilog\bin\iverilog.exe

	     The IVL_ROOT in a Windows installation is the path:

	         D:\iverilog\lib\ivl

	     so we chop the file name and the last directory by
	     turning the last two \ characters to null. Then we append
	     the lib\ivl to finish. */

        strncpy(ivl_root, basepath, MAXSIZE);
	s = strrchr(ivl_root, sep);
	if (s) *s = 0;
	s = strrchr(ivl_root, sep);
	if (s) *s = 0;
	strcat(ivl_root, "\\lib\\ivl");
      }
#else

      { char*var = 0;

	  /* In other systems, use the configured root. */
        strcpy(ivl_root, IVL_ROOT);

	  /* In any case, the IVL_ROOT variable can be used to override
	     the install location at run time. If it is set, use that
	     value instead. */
	if ( (var = getenv(IVL_ROOT_VARIABLE)) ) {
	      strncpy(ivl_root, var, MAXSIZE);
	      strcat(ivl_root, "/lib/ivl");
	}
      }
#endif

      return ivl_root;
}

int main(int argc, char **argv)
{
      char*cmd;
      unsigned ncmd;
      int e_flag = 0;
      int version_flag = 0;
      int opt, idx, rc;

      base = get_root_dir();

	/* Create a temporary file for communicating input parameters
	   to the preprocessor. */
      source_path = strdup(my_tempfile("ivrlg", &source_file));
      if (NULL == source_file) {
	    fprintf(stderr, "%s: Error opening temporary file %s\n",
		    argv[0], source_path);
	    fprintf(stderr, "%s: Please check TMP or TMPDIR.\n", argv[0]);
	    return 1;
      }

	/* Create another temporary file for passing configuration
	   information to ivl. */

      if ( (iconfig_path = getenv("IVERILOG_ICONFIG")) ) {
	    fprintf(stderr, "%s: IVERILOG_ICONFIG=%s\n",
		    argv[0], iconfig_path);

	    iconfig_file = fopen(iconfig_path, "w");

      } else {

	    iconfig_path = strdup(my_tempfile("ivrlh", &iconfig_file));
      }

      if (NULL == iconfig_file) {
	    fprintf(stderr, "%s: Error opening temporary file %s\n",
		    argv[0], iconfig_path);
	    fprintf(stderr, "%s: Please check TMP or TMPDIR.\n", argv[0]);
	    fclose(source_file);
	    remove(source_path);
	    return 1;
      }

      while ((opt = getopt(argc, argv, "B:c:D:Ef:g:hI:M:m:N::o:p:Ss:T:t:vVW:y:Y:")) != EOF) {

	    switch (opt) {
		case 'B':
		    /* Undocumented feature: The preprocessor itself
		       may be located at a different location. If the
		       base starts with a 'P', set this special base
		       instead of the main base. */
		  if (optarg[0] == 'P') {
			pbase = optarg+1;
		  } else {
			base=optarg;
		  }
		  break;
 		case 'c':
		  command_filename = malloc(strlen(optarg)+1);
 		  strcpy(command_filename, optarg);
 		  break;
		case 'D':
		  process_define(optarg);
		  break;
		case 'E':
		  e_flag = 1;
		  break;
		case 'f':
		  fprintf(stderr, "warning: The -f flag is moved to -p\n");
		case 'p':
		  fprintf(iconfig_file, "flag:%s\n", optarg);
		  break;

		case 'g':
		  rc = process_generation(optarg);
		  if (rc != 0)
			return -1;
		  break;
 		case 'h':
 		  fprintf(stderr, "%s\n", HELP);
 		  return 1;

		case 'I':
		  process_include_dir(optarg);
		  break;

		case 'M':
		  depfile = optarg;
		  break;

		case 'm':
		  fprintf(iconfig_file, "module:%s\n", optarg);
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
		  fprintf(iconfig_file, "root:%s\n", optarg);
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
		case 'V':
		  version_flag = 1;
		  break;
		case 'W':
		  process_warning_switch(optarg);
		  break;
		case 'y':
		  process_library_switch(optarg);
		  break;
		case 'Y':
		  process_library2_switch(optarg);
		  break;
		case '?':
		default:
		  return 1;
	    }
      }

      if (pbase == 0)
	    pbase = base;

      if (version_flag || verbose_flag) {
	    printf("Icarus Verilog version " VERSION "\n\n");
	    printf("Copyright 1998-2009 Stephen Williams\n");
	    puts(NOTICE);

	    if (version_flag)
		  return 0;
      }

	/* Make a common conf file path to reflect the target. */
      sprintf(iconfig_common_path, "%s%c%s%s.conf",
	      base,sep, targ, synth_flag? "-s" : "");

	/* Write values to the iconfig file. */
      fprintf(iconfig_file, "basedir:%s\n", base);

	/* Tell the core where to find the system.sft. This file
	   describes the system functions so that elaboration knows
	   how to handle them. */
      fprintf(iconfig_file, "sys_func:%s%csystem.sft\n", base, sep);

      if (mtm != 0) fprintf(iconfig_file, "-T:%s\n", mtm);
      fprintf(iconfig_file, "generation:%s\n", generation);
      fprintf(iconfig_file, "warnings:%s\n", warning_flags);
      fprintf(iconfig_file, "out:%s\n", opath);
      if (depfile) fprintf(iconfig_file, "depfile:%s\n", depfile);

      if (command_filename) {
	    int rc;

	    if (( fp = fopen(command_filename, "r")) == NULL ) {
		  fprintf(stderr, "%s: Can't open %s\n",
			  argv[0], command_filename);
		  return 1;
	    }

	    cfreset(fp, command_filename);
	    rc = cfparse();
	    if (rc != 0) {
		  fprintf(stderr, "%s: error reading command file\n",
			  command_filename);
		  return 1;
	    }
      }

	/* Finally, process all the remaining words on the command
	   line as file names. */
      for (idx = optind ;  idx < argc ;  idx += 1)
	    process_file_name(argv[idx]);


      fclose(source_file);
      source_file = 0;

      if (source_count == 0) {
	    fprintf(stderr, "%s: No input files.\n", argv[0]);
 	    fprintf(stderr, "%s\n", HELP);
	    return 1;
      }


	/* Start building the preprocess command line. */

      sprintf(tmp, "%s%civlpp %s%s -D__ICARUS__=1 -f%s ", pbase,sep,
	      verbose_flag?" -v":"",
	      e_flag?"":" -L", source_path);

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

      if (depfile) {
	    cmd = realloc(cmd, ncmd + strlen(depfile) + 5);
	    strcat(cmd, " -M ");
	    strcat(cmd, depfile);
	    ncmd += strlen(depfile) + 4;
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
	    remove(source_path);
	    fclose(iconfig_file);
	    if ( ! getenv("IVERILOG_ICONFIG"))
		  remove(iconfig_path);

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

	/* Write the preprocessor command needed to preprocess a
	   single file. This may be used to preprocess library
	   files. */
      fprintf(iconfig_file, "ivlpp:%s%civlpp -D__ICARUS__ -L %s %s\n",
	      pbase, sep,
	      inc_list? inc_list : "",
	      def_list? def_list : "");

	/* Done writing to the iconfig file. Close it now. */
      fclose(iconfig_file);

      return t_default(cmd, ncmd);

      return 0;
}

/*
 * $Log: main.c,v $
 * Revision 1.65.2.6  2007/05/30 17:48:26  steve
 *  DEBUG aids. (Alan Feldstein)
 *
 * Revision 1.65.2.5  2006/07/07 21:31:50  steve
 *  Root dir variable does not include lib/ivl components.
 *
 * Revision 1.65.2.4  2006/06/27 01:30:20  steve
 *  Fix unused var warning for mingw32 build.
 *
 * Revision 1.65.2.3  2006/06/14 03:01:49  steve
 *  Remove redundant call to get_root_dir.
 *
 * Revision 1.65.2.2  2006/06/12 00:16:53  steve
 *  Add support for -Wunused warnings.
 *
 * Revision 1.65.2.1  2006/03/26 21:47:26  steve
 *  More installation directory flexibility.
 *
 * Revision 1.65  2004/06/17 14:47:22  steve
 *  Add a .sft file for the system functions.
 *
 * Revision 1.64  2004/03/10 04:51:25  steve
 *  Add support for system function table files.
 *
 * Revision 1.63  2004/02/15 18:03:30  steve
 *  Cleanup of warnings.
 *
 * Revision 1.62  2003/12/12 04:36:48  steve
 *  Fix make check to support -tconf configuration method.
 *
 * Revision 1.61  2003/11/18 06:31:46  steve
 *  Remove the iverilog.conf file.
 *
 * Revision 1.60  2003/11/13 05:55:33  steve
 *  Move the DLL= flag to target config files.
 *
 * Revision 1.59  2003/11/13 04:09:49  steve
 *  Pass flags through the temporary config file.
 *
 * Revision 1.58  2003/11/01 04:21:57  steve
 *  Add support for a target static config file.
 *
 * Revision 1.57  2003/10/26 22:43:42  steve
 *  Improve -V messages,
 *
 * Revision 1.56  2003/09/26 21:25:58  steve
 *  Warnings cleanup.
 *
 * Revision 1.55  2003/09/23 05:57:15  steve
 *  Pass -m flag from driver via iconfig file.
 *
 * Revision 1.54  2003/09/22 01:12:09  steve
 *  Pass more ivl arguments through the iconfig file.
 *
 * Revision 1.53  2003/08/26 16:26:02  steve
 *  ifdef idents correctly.
 *
 * Revision 1.52  2003/02/22 04:55:36  steve
 *  portbind adds p, not i, flag.
 *
 * Revision 1.51  2003/02/22 04:12:49  steve
 *  Add the portbind warning.
 */

