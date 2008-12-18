/*
 * Copyright (c) 2000-2008 Stephen Williams (steve@icarus.com)
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
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;

const char HELP[] =
"Usage: iverilog [-ESvV] [-B base] [-c cmdfile|-f cmdfile] [-g1|-g2|-g2x]\n"
"                [-D macro[=defn]] [-I includedir] [-M depfile] [-m module]\n"
"                [-N file] [-o filename] [-p flag=value]\n"
"                [-s topmodule] [-t target] [-T min|typ|max]\n"
"                [-W class] [-y dir] [-Y suf] source_file(s)\n"
"\n"
"See the man page for details.";

#define MAXSIZE 4096

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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

const char*generation = "2005";
const char*gen_specify = "no-specify";
const char*gen_xtypes = "xtypes";
const char*gen_icarus = "icarus-misc";
const char*gen_io_range_error = "io-range-error";
const char*gen_verilog_ams = "no-verilog-ams";

/* Boolean: true means use a default include dir, false means don't */
int gen_std_include = 1;

char warning_flags[16] = "";

unsigned integer_width = 32;

char*mod_list = 0;
char*command_filename = 0;

/* These are used to collect the list of file names that will be
   passed to ivlpp. Keep the list in a file because it can be a long
   list. */
char*source_path = 0;
FILE*source_file = 0;
unsigned source_count = 0;

char*defines_path = 0;
FILE*defines_file = 0;

char*iconfig_path = 0;
FILE*iconfig_file = 0;

char*compiled_defines_path = 0;

static char iconfig_common_path_buf[4096] = "";
char*iconfig_common_path = iconfig_common_path_buf;

int synth_flag = 0;
int verbose_flag = 0;

FILE *fp;

char line[MAXSIZE];
char tmp[MAXSIZE];

static char ivl_root[MAXSIZE];

/* Structure to keep a FIFO list of the command files */
typedef struct t_command_file {
      char *filename;
      struct t_command_file *next;
} s_command_file, *p_command_file;
p_command_file cmd_file_head = NULL;  /* The FIFO head */
p_command_file cmd_file_tail = NULL;  /* The FIFO tail */

/* Function to add a command file name to the FIFO. */
void add_cmd_file(const char* filename)
{
      p_command_file new;

      new = (p_command_file) malloc(sizeof(s_command_file));
      new->filename = strdup(filename);
      new->next = NULL;
      if (cmd_file_head == NULL) {
            cmd_file_head = new;
            cmd_file_tail = new;
      } else {
            cmd_file_tail->next = new;
            cmd_file_tail = new;
      }
}

/* Function to return the top command file name from the FIFO. */
char *get_cmd_file()
{
      char *filename;

      if (cmd_file_head == NULL) filename = NULL;
      else {
            p_command_file head;

            filename = cmd_file_head->filename;
            head = cmd_file_head;
            cmd_file_head = cmd_file_head->next;
            free(head);
      }
      return filename;
}

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

static int t_version_only(void)
{
      int rc;
      remove(source_path);
      free(source_path);

      fflush(0);
      snprintf(tmp, sizeof tmp, "%s%civlpp -V", pbase, sep);
      rc = system(tmp);
      if (rc != 0) {
	    fprintf(stderr, "Unable to get version from \"%s\"\n", tmp);
      }

      fflush(0);
      snprintf(tmp, sizeof tmp, "%s%civl -V -C%s -C%s", pbase, sep,
	       iconfig_path, iconfig_common_path);
      rc = system(tmp);
      if (rc != 0) {
	    fprintf(stderr, "Unable to get version from \"%s\"\n", tmp);
      }

      if ( ! getenv("IVERILOG_ICONFIG")) {
	    remove(iconfig_path);
	    free(iconfig_path);
	    remove(defines_path);
	    free(defines_path);
	    remove(compiled_defines_path);
	    free(compiled_defines_path);
      }

      return 0;
}

static void build_preprocess_command(int e_flag)
{
      snprintf(tmp, sizeof tmp, "%s%civlpp %s%s -F%s -f%s -p%s ",
	       pbase,sep, verbose_flag?" -v":"",
	       e_flag?"":" -L", defines_path, source_path,
	       compiled_defines_path);
}

static int t_preprocess_only(void)
{
      int rc;
      char*cmd;
      unsigned ncmd;

      build_preprocess_command(1);

      ncmd = strlen(tmp);
      cmd = malloc(ncmd+1);
      strcpy(cmd, tmp);

      if (strcmp(opath,"-") != 0) {
	    snprintf(tmp, sizeof tmp, " > %s", opath);
	    cmd = realloc(cmd, ncmd+strlen(tmp)+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += strlen(tmp);
      }

      if (verbose_flag)
	    printf("preprocess: %s\n", cmd);

      rc = system(cmd);
      remove(source_path);
      free(source_path);

      if ( ! getenv("IVERILOG_ICONFIG")) {
	    remove(iconfig_path);
	    free(iconfig_path);
	    remove(defines_path);
	    free(defines_path);
	    remove(compiled_defines_path);
	    free(compiled_defines_path);
      }

      if (rc != 0) {
	    if (WIFEXITED(rc)) {
		  fprintf(stderr, "errors preprocessing Verilog program.\n");
		  return WEXITSTATUS(rc);
	    }

	    fprintf(stderr, "Command signaled: %s\n", cmd);
	    free(cmd);
	    return -1;
      }

      free(cmd);
      return 0;
}

/*
 * This is the default target type. It looks up the bits that are
 * needed to run the command from the configuration file (which is
 * already parsed for us) so we can handle must of the generic cases.
 */
static int t_compile()
{
      unsigned rc;

	/* Start by building the preprocess command line. */
      build_preprocess_command(0);

      size_t ncmd = strlen(tmp);
      char*cmd = malloc(ncmd + 1);
      strcpy(cmd, tmp);

#ifdef __MINGW32__
      unsigned ncmd_start = ncmd;
#else
      int rtn;
#endif

	/* Build the ivl command and pipe it to the preprocessor. */
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


      if (verbose_flag)
	    printf("translate: %s\n", cmd);


      rc = system(cmd);
      if ( ! getenv("IVERILOG_ICONFIG")) {
	    remove(source_path);
	    free(source_path);
	    remove(iconfig_path);
	    free(iconfig_path);
	    remove(defines_path);
	    free(defines_path);
	    remove(compiled_defines_path);
	    free(compiled_defines_path);
      }
#ifdef __MINGW32__  /* MinGW just returns the exit status, so return it! */
      free(cmd);
      return rc;
#else
      rtn = 0;
      if (rc != 0) {
	    if (rc == 127) {
		  fprintf(stderr, "Failed to execute: %s\n", cmd);
		  rtn = 1;
	    } else if (WIFEXITED(rc)) {
		  rtn = WEXITSTATUS(rc);
	    } else {
		  fprintf(stderr, "Command signaled: %s\n", cmd);
		  rtn = -1;
	    }
      }

      free(cmd);
      return rtn;
#endif
}


static void process_warning_switch(const char*name)
{
      if (strcmp(name,"all") == 0) {
	    process_warning_switch("implicit");
	    process_warning_switch("portbind");
	    process_warning_switch("timescale");
      } else if (strcmp(name,"implicit") == 0) {
	    if (! strchr(warning_flags, 'i'))
		  strcat(warning_flags, "i");
      } else if (strcmp(name,"portbind") == 0) {
	    if (! strchr(warning_flags, 'p'))
		  strcat(warning_flags, "p");
      } else if (strcmp(name,"timescale") == 0) {
	    if (! strchr(warning_flags, 't'))
		  strcat(warning_flags, "t");
	/* Since the infinite loop check is not part of 'all' it
	 * does not have a no- version. */
      } else if (strcmp(name,"infloop") == 0) {
	    if (! strchr(warning_flags, 'l'))
		  strcat(warning_flags, "l");
      } else if (strcmp(name,"no-implicit") == 0) {
	    char*cp = strchr(warning_flags, 'i');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-portbind") == 0) {
	    char*cp = strchr(warning_flags, 'p');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-timescale") == 0) {
	    char*cp = strchr(warning_flags, 't');
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
      fprintf(defines_file, "I:%s\n", name);
}

void process_define(const char*name)
{
      fprintf(defines_file,"D:%s\n", name);
}

/*
 * This function is called while processing a file name in a command
 * file, or a file name on the command line. Look to see if there is a
 * .sft suffix, and if so pass that as a sys_func file. Otherwise, it
 * is a Verilog source file to be written into the file list.
 */
void process_file_name(const char*name, int lib_flag)
{
      if (strlen(name) > 4 && strcasecmp(".sft", name+strlen(name)-4) == 0) {
	    fprintf(iconfig_file,"sys_func:%s\n", name);

      } else {
	    fprintf(source_file, "%s\n", name);
	    source_count += 1;
	    if (lib_flag)
		  fprintf(iconfig_file,"library_file:%s\n", name);
      }
}

int process_generation(const char*name)
{
      if (strcmp(name,"1995") == 0)
	    generation = "1995";

      else if (strcmp(name,"2001") == 0)
	    generation = "2001";

      else if (strcmp(name,"2005") == 0)
	    generation = "2005";

      else if (strcmp(name,"1") == 0) { /* Deprecated: use 1995 */
	    generation = "1995";
	    gen_xtypes = "no-xtypes";
	    gen_icarus = "no-icarus-misc";

      } else if (strcmp(name,"2") == 0) { /* Deprecated: use 2001 */
	    generation = "2001";
	    gen_xtypes = "no-xtypes";
	    gen_icarus = "no-icarus-misc";

      } else if (strcmp(name,"2x") == 0) { /* Deprecated: use 2005/xtypes */
	    generation = "2005";
	    gen_xtypes = "xtypes";
	    gen_icarus = "icarus-misc";

      } else if (strcmp(name,"xtypes") == 0)
	    gen_xtypes = "xtypes";

      else if (strcmp(name,"no-xtypes") == 0)
	    gen_xtypes = "no-xtypes";

      else if (strcmp(name,"icarus-misc") == 0)
	    gen_icarus = "icarus-misc";

      else if (strcmp(name,"no-icarus-misc") == 0)
	    gen_icarus = "no-icarus-misc";

      else if (strcmp(name,"specify") == 0)
	    gen_specify = "specify";

      else if (strcmp(name,"no-specify") == 0)
	    gen_specify = "no-specify";

      else if (strcmp(name,"std-include") == 0)
	     gen_std_include = 1;

      else if (strcmp(name,"no-std-include") == 0)
	     gen_std_include = 0;

      else if (strcmp(name,"io-range-error") == 0)
	    gen_io_range_error = "io-range-error";

      else if (strcmp(name,"no-io-range-error") == 0)
	    gen_io_range_error = "no-io-range-error";

      else if (strcmp(name,"verilog-ams") == 0)
	    gen_verilog_ams = "verilog-ams";

      else if (strcmp(name,"no-verilog-ams") == 0)
	    gen_verilog_ams = "no-verilog-ams";

      else {
	    fprintf(stderr, "Unknown/Unsupported Language generation "
		    "%s\n\n", name);
	    fprintf(stderr, "Supported generations are:\n");
	    fprintf(stderr, "    1995 -- IEEE1364-1995\n"
		            "    2001 -- IEEE1364-2001\n"
		            "    2005 -- IEEE1364-2005\n"
		            "Other generation flags:\n"
		            "    specify | no-specify\n"
		            "    verilog-ams | no-verilog-ams\n"
		            "    std-include | no-std-include\n"
		            "    xtypes | no-xtypes\n"
		            "    icarus-misc | no-icarus-misc\n"
		            "    io-range-error | no-io-range-error\n");
	    return 1;
      }

      return 0;
}

/*
 * If it exists add the SFT file for the given module.
 */
void add_sft_file(const char *module)
{
      char *file;

      file = (char *) malloc(strlen(base)+1+strlen(module)+4+1);
      sprintf(file, "%s%c%s.sft", base, sep, module);
      if (access(file, R_OK) == 0)
            fprintf(iconfig_file, "sys_func:%s\n", file);
      free(file);
}

int main(int argc, char **argv)
{
      int e_flag = 0;
      int version_flag = 0;
      int opt, idx;

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

	base = ivl_root;
      }

#else
        /* In a UNIX environment, the IVL_ROOT from the Makefile is
	   dependable. It points to the $prefix/lib/ivl directory,
	   where the sub-parts are installed. */
      strcpy(ivl_root, IVL_ROOT);
      base = ivl_root;
#endif

	/* Create a temporary file for communicating input parameters
	   to the preprocessor. */
      source_path = strdup(my_tempfile("ivrlg", &source_file));
      if (NULL == source_file) {
	    fprintf(stderr, "%s: Error opening temporary file %s\n",
		    argv[0], source_path);
	    fprintf(stderr, "%s: Please check TMP or TMPDIR.\n", argv[0]);
	    return 1;
      }

      defines_path = strdup(my_tempfile("ivrlg2", &defines_file));
      if (NULL == defines_file) {
	    fprintf(stderr, "%s: Error opening temporary file %s\n",
		    argv[0], defines_path);
	    fprintf(stderr, "%s: Please check TMP or TMPDIR.\n", argv[0]);

	    fclose(source_file);
	    remove(source_path);
	    return 1;
      }

      fprintf(defines_file, "D:__ICARUS__=1\n");
      if (strcmp(gen_verilog_ams,"verilog-ams") == 0)
	    fprintf(defines_file, "D:__VAMS_ENABLE__=1\n");

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

	    fclose(defines_file);
	    remove(defines_path);
	    return 1;
      }

	/* Create a temporary file (I only really need the path) that
	   can carry preprocessor precompiled defines to the library. */
      { FILE*tmp_file = 0;
	compiled_defines_path = strdup(my_tempfile("ivrli", &tmp_file));
	if (tmp_file) {
	      fclose(tmp_file);
	}
      }

      while ((opt = getopt(argc, argv, "B:c:D:d:Ef:g:hI:M:m:N::o:p:Ss:T:t:vVW:y:Y:")) != EOF) {

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
		case 'f':
		  add_cmd_file(optarg);
		  break;
		case 'D':
		  process_define(optarg);
		  break;
		case 'E':
		  e_flag = 1;
		  break;
		case 'p':
		  fprintf(iconfig_file, "flag:%s\n", optarg);
		  break;
		case 'd':
		  fprintf(iconfig_file, "debug:%s\n", optarg);
		  break;

		case 'g':
		  if (process_generation(optarg) != 0)
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
		  add_sft_file(optarg);
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
	    printf("Icarus Verilog version " VERSION " (" VERSION_TAG ")\n\n");
	    printf("Copyright 1998-2008 Stephen Williams\n\n");
	    puts(NOTICE);
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

	/* If verilog-2005 is enabled or icarus-misc or verilog-ams,
	 * then include the v2005_math library. */
      if (strcmp(generation, "2005") == 0 ||
          strcmp(gen_icarus, "icarus-misc") == 0 ||
          strcmp(gen_verilog_ams, "verilog-ams") == 0) {
	    fprintf(iconfig_file, "sys_func:%s%cv2005_math.sft\n", base, sep);
	    fprintf(iconfig_file, "module:v2005_math\n");
      }
	/* If verilog-ams or icarus_misc is enabled, then include the
	 * va_math module as well. */
      if (strcmp(gen_verilog_ams,"verilog-ams") == 0 ||
          strcmp(gen_icarus, "icarus-misc") == 0) {
	    fprintf(iconfig_file, "sys_func:%s%cva_math.sft\n", base, sep);
	    fprintf(iconfig_file, "module:va_math\n");
      }

      if (mtm != 0) fprintf(iconfig_file, "-T:%s\n", mtm);
      fprintf(iconfig_file, "generation:%s\n", generation);
      fprintf(iconfig_file, "generation:%s\n", gen_specify);
      fprintf(iconfig_file, "generation:%s\n", gen_xtypes);
      fprintf(iconfig_file, "generation:%s\n", gen_io_range_error);
      fprintf(iconfig_file, "generation:%s\n", gen_verilog_ams);
      fprintf(iconfig_file, "generation:%s\n", gen_icarus);
      fprintf(iconfig_file, "warnings:%s\n", warning_flags);
      fprintf(iconfig_file, "out:%s\n", opath);
      if (depfile) fprintf(iconfig_file, "depfile:%s\n", depfile);

      while ( (command_filename = get_cmd_file()) ) {
	    int rc;

	    if (( fp = fopen(command_filename, "r")) == NULL ) {
		  fprintf(stderr, "%s: cannot open command file %s "
			  "for reading.\n", argv[0], command_filename);
		  return 1;
	    }

	    cfreset(fp, command_filename);
	    rc = cfparse();
	    if (rc != 0) {
		  fprintf(stderr, "%s: parsing failed in base command "
		          "file %s.\n", argv[0], command_filename);
		  return 1;
	    }
            free(command_filename);
      }

      if (depfile) {
	    fprintf(defines_file, "M:%s\n", depfile);
      }

	/* Finally, process all the remaining words on the command
	   line as file names. */
      for (idx = optind ;  idx < argc ;  idx += 1)
	    process_file_name(argv[idx], 0);

	/* If the use of a default include directory is not
	   specifically disabled, then write that directory as the
	   very last include directory to use... always. */
      if (gen_std_include) {
	    fprintf(defines_file, "I:%s%cinclude", base, sep);
      }

      fclose(source_file);
      source_file = 0;

      fclose(defines_file);
      defines_file = 0;

      if (source_count == 0 && !version_flag) {
	    fprintf(stderr, "%s: no source files.\n\n%s\n", argv[0], HELP);
	    return 1;
      }

      fprintf(iconfig_file, "iwidth:%u\n", integer_width);

	/* Write the preprocessor command needed to preprocess a
	   single file. This may be used to preprocess library
	   files. */
      fprintf(iconfig_file, "ivlpp:%s%civlpp -L -F%s -P%s\n",
	      pbase, sep, defines_path, compiled_defines_path);

	/* Done writing to the iconfig file. Close it now. */
      fclose(iconfig_file);

	/* If we're only here for the version output, then we're done. */
      if (version_flag)
	    return t_version_only();

	/* If the -E flag was given on the command line, then all we
	   do is run the preprocessor and put the output where the
	   user wants it. */
      if (e_flag)
	    return t_preprocess_only();

	/* Otherwise, this is a full compile. */
      return t_compile();
}
