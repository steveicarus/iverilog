const char COPYRIGHT[] =
  "Copyright (c) 2000-2024 Stephen Williams (steve@icarus.com)";
/*
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"
# include "version_base.h"
# include "version_tag.h"

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
"Usage: iverilog [-EiRSuvV] [-B base] [-c cmdfile|-f cmdfile]\n"
"                [-g1995|-g2001|-g2005|-g2005-sv|-g2009|-g2012] [-g<feature>]\n"
"                [-D macro[=defn]] [-I includedir] [-L moduledir]\n"
"                [-M [mode=]depfile] [-m module]\n"
"                [-N file] [-o filename] [-p flag=value]\n"
"                [-s topmodule] [-t target] [-T min|typ|max]\n"
"                [-W class] [-y dir] [-Y suf] [-l file] source_file(s)\n"
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
# include  <windows.h>
# include  <io.h>
#ifdef HAVE_LIBIBERTY_H
# include  <libiberty.h>
#endif
#endif
#include  <fcntl.h>

#ifdef HAVE_GETOPT_H
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
#include "ivl_alloc.h"

#ifdef __MINGW32__
const char sep = '\\';
const char path_sep = ';';
#else
const char sep = '/';
const char path_sep = ':';
#endif

extern void cfreset(FILE*fd, const char*path);

const char*base = 0;
const char*vpi_dir = 0;
const char*ivlpp_dir = 0;
const char*vhdlpp_dir= 0;
const char*vhdlpp_work = 0;
const char*mtm  = 0;
const char*opath = "a.out";
const char*npath = 0;
const char*targ  = "vvp";
const char*depfile = 0;

const char**vhdlpp_libdir = 0;
unsigned vhdlpp_libdir_cnt = 0;

char depmode = 'a';

const char*generation = "2005";
const char*gen_specify = "no-specify";
const char*gen_interconnect = "no-interconnect";
const char*gen_assertions = "assertions";
const char*gen_xtypes = "xtypes";
const char*gen_icarus = "icarus-misc";
const char*gen_io_range_error = "io-range-error";
const char*gen_strict_ca_eval = "no-strict-ca-eval";
const char*gen_strict_expr_width = "no-strict-expr-width";
const char*gen_shared_loop_index = "shared-loop-index";
const char*gen_verilog_ams = "no-verilog-ams";

/* Boolean: true means use a default include dir, false means don't */
int gen_std_include = 1;

/* Boolean: true means add the local file directory to the start
   of the include list. */
int gen_relative_include = 0;

char warning_flags[17] = "n";

int separate_compilation_flag = 0;

/* Boolean: true means ignore errors about missing modules */
int ignore_missing_modules = 0;

unsigned integer_width = 32;

unsigned width_cap = 65536;

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

static char iconfig_common_path[4096] = "";

static const char**vpi_path_list = 0;
static unsigned vpi_path_list_size = 0;

static const char**env_vpi_path_list = 0;
static unsigned env_vpi_path_list_size = 0;

int synth_flag = 0;
int verbose_flag = 0;

FILE *fp;

char line[MAXSIZE];
char tmp[MAXSIZE+24];

static char ivl_root[MAXSIZE];

/* Structure to keep a FIFO list of the command files */
typedef struct t_command_file {
      char *filename;
      struct t_command_file *next;
} s_command_file, *p_command_file;
p_command_file cmd_file_head = NULL;  /* The FIFO head */
p_command_file cmd_file_tail = NULL;  /* The FIFO tail */

/* Temporarily store parameter definition from command line and
 * parse it after we have dealt with command file
 */
static const char** defparm_base = 0;
static int defparm_size = 0;

/* Function to add a command file name to the FIFO. */
static void add_cmd_file(const char* filename)
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
static char *get_cmd_file(void)
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

#ifdef __MINGW32__
/*
 * The MinGW version of getenv() returns the path with a forward
 * slash. This should be converted to a back slash to keep every
 * thing in the code using a back slash. This function wraps the
 * code for this in one place. The conversion can not be done
 * directly on the getenv() result since it is const char*.
 */
static void convert_to_MS_path(char *path)
{
      char *t;
      for (t = path; *t; t++) {
	    if (*t == '/') *t = '\\';
      }
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
	    snprintf(pathbuf, sizeof pathbuf, "%s%c%s%04x",
	             tmpdir, sep, str, code);
#ifdef __MINGW32__
	    convert_to_MS_path(pathbuf);
#endif
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
      snprintf(tmp, sizeof tmp, "%s%civlpp -V", ivlpp_dir, sep);
      rc = system(tmp);
      if (rc != 0) {
	    fprintf(stderr, "Unable to get version from \"%s\"\n", tmp);
      }

      fflush(0);
      snprintf(tmp, sizeof tmp, "%s%civl -V -C\"%s\" -C\"%s\"", base, sep,
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
      snprintf(tmp, sizeof tmp, "%s%civlpp%s%s%s -F\"%s\" -f\"%s\" -p\"%s\"%s",
	       ivlpp_dir, sep,
               verbose_flag ? " -v" : "",
	       e_flag ? "" : " -L",
               strchr(warning_flags, 'r') ? " -Wredef-all" :
               strchr(warning_flags, 'R') ? " -Wredef-chg" : "",
               defines_path, source_path,
	       compiled_defines_path,
	       e_flag ? "" : " |");
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
	    snprintf(tmp, sizeof tmp, " > \"%s\"", opath);
	    cmd = realloc(cmd, ncmd+strlen(tmp)+1);
	    strcpy(cmd+ncmd, tmp);
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
		  free(cmd);
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
static int t_compile(void)
{
      unsigned rc;

	/* Start by building the preprocess command line, if required.
	   This pipes into the main ivl command. */
      if (!separate_compilation_flag)
	    build_preprocess_command(0);
      else
	    strcpy(tmp, "");

      size_t ncmd = strlen(tmp);
      char*cmd = malloc(ncmd + 1);
      strcpy(cmd, tmp);

#ifndef __MINGW32__
      int rtn;
#endif

	/* Build the ivl command. */
      snprintf(tmp, sizeof tmp, "%s%civl", base, sep);
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
	    snprintf(tmp, sizeof tmp, " -N\"%s\"", npath);
	    rc = strlen(tmp);
	    cmd = realloc(cmd, ncmd+rc+1);
	    strcpy(cmd+ncmd, tmp);
	    ncmd += rc;
      }

      snprintf(tmp, sizeof tmp, " -C\"%s\"", iconfig_path);
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      snprintf(tmp, sizeof tmp, " -C\"%s\"", iconfig_common_path);
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);
      ncmd += rc;

      if (separate_compilation_flag)
	    snprintf(tmp, sizeof tmp, " -F\"%s\"", source_path);
      else
	    snprintf(tmp, sizeof tmp, " -- -");
      rc = strlen(tmp);
      cmd = realloc(cmd, ncmd+rc+1);
      strcpy(cmd+ncmd, tmp);


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
	    process_warning_switch("anachronisms");
	    process_warning_switch("implicit");
	    process_warning_switch("implicit-dimensions");
	    process_warning_switch("macro-replacement");
	    process_warning_switch("portbind");
	    process_warning_switch("select-range");
	    process_warning_switch("timescale");
	    process_warning_switch("sensitivity-entire-array");
      } else if (strcmp(name,"anachronisms") == 0) {
	    if (! strchr(warning_flags, 'n'))
		  strcat(warning_flags, "n");
      } else if (strcmp(name,"floating-nets") == 0) {
	    if (! strchr(warning_flags, 'f'))
		  strcat(warning_flags, "f");
      } else if (strcmp(name,"implicit") == 0) {
	    if (! strchr(warning_flags, 'i'))
		  strcat(warning_flags, "i");
      } else if (strcmp(name,"implicit-dimensions") == 0) {
	    if (! strchr(warning_flags, 'd'))
		  strcat(warning_flags, "d");
      } else if (strcmp(name,"macro-redefinition") == 0) {
	  if (! strchr(warning_flags, 'r'))
		  strcat(warning_flags, "r");
      } else if (strcmp(name,"macro-replacement") == 0) {
	  if (! strchr(warning_flags, 'R'))
		  strcat(warning_flags, "R");
      } else if (strcmp(name,"portbind") == 0) {
	    if (! strchr(warning_flags, 'p'))
		  strcat(warning_flags, "p");
      } else if (strcmp(name,"select-range") == 0) {
	    if (! strchr(warning_flags, 's'))
		  strcat(warning_flags, "s");
      } else if (strcmp(name,"timescale") == 0) {
	    if (! strchr(warning_flags, 't'))
		  strcat(warning_flags, "t");
	/* Since the infinite loop check is not part of 'all' it
	 * does not have a no- version. */
      } else if (strcmp(name,"infloop") == 0) {
	    if (! strchr(warning_flags, 'l'))
		  strcat(warning_flags, "l");
      } else if (strcmp(name,"sensitivity-entire-vector") == 0) {
	    if (! strchr(warning_flags, 'v'))
		  strcat(warning_flags, "v");
      } else if (strcmp(name,"sensitivity-entire-array") == 0) {
	    if (! strchr(warning_flags, 'a'))
		  strcat(warning_flags, "a");
      } else if (strcmp(name,"no-anachronisms") == 0) {
	    char*cp = strchr(warning_flags, 'n');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-floating-nets") == 0) {
	    char*cp = strchr(warning_flags, 'f');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-implicit") == 0) {
	    char*cp = strchr(warning_flags, 'i');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-implicit-dimensions") == 0) {
	    char*cp = strchr(warning_flags, 'd');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-macro-redefinition") == 0) {
	    char*cp = strchr(warning_flags, 'r');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
	    cp = strchr(warning_flags, 'R');
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
      } else if (strcmp(name,"no-select-range") == 0) {
	    char*cp = strchr(warning_flags, 's');
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
      } else if (strcmp(name,"no-sensitivity-entire-vector") == 0) {
	    char*cp = strchr(warning_flags, 'v');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else if (strcmp(name,"no-sensitivity-entire-array") == 0) {
	    char*cp = strchr(warning_flags, 'a');
	    if (cp) while (*cp) {
		  cp[0] = cp[1];
		  cp += 1;
	    }
      } else {
	    fprintf(stderr, "Ignoring unknown warning class "
		    "%s\n", name);
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

void process_parameter(const char*name)
{
      fprintf(iconfig_file,"defparam:%s\n", name);
}

void process_timescale(const char*ts_string)
{
      fprintf(iconfig_file, "timescale:%s\n", ts_string);
}

/*
 * This function is called while processing a file name in a command
 * file, or a file name on the command line. Look to see if there is a
 * .sft or .vpi suffix, and if so pass that as a sys_func or module
 * file. Otherwise, it is a Verilog source file to be written into the
 * file list.
 */
void process_file_name(const char*name, int lib_flag)
{
      if (strlen(name) > 4 && strcasecmp(".sft", name+strlen(name)-4) == 0) {
	    fprintf(stderr, "SFT files are deprecated. Please pass the VPI module instead.\n");
	    fprintf(iconfig_file,"sys_func:%s\n", name);

      } else if (strlen(name) > 4 && strcasecmp(".vpi", name+strlen(name)-4) == 0) {
	    fprintf(iconfig_file,"module:%s\n", name);

      } else {
	    fprintf(source_file, "%s\n", name);
	    source_count += 1;
	    if (lib_flag)
		  fprintf(iconfig_file,"library_file:%s\n", name);
      }
}

static int process_generation(const char*name)
{
      if (strcmp(name,"1995") == 0)
	    generation = "1995";

      else if (strcmp(name,"2001") == 0)
	    generation = "2001";

      else if (strcmp(name,"2001-noconfig") == 0)
	    generation = "2001-noconfig";

      else if (strcmp(name,"2005") == 0)
	    generation = "2005";

      else if (strcmp(name,"2005-sv") == 0)
	    generation = "2005-sv";

      else if (strcmp(name,"2009") == 0)
	    generation = "2009";

      else if (strcmp(name,"2012") == 0)
	    generation = "2012";

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

      else if (strcmp(name,"interconnect") == 0)
	    gen_interconnect = "interconnect";

      else if (strcmp(name,"no-interconnect") == 0)
	    gen_interconnect = "no-interconnect";

      else if (strcmp(name,"assertions") == 0)
	    gen_assertions = "assertions";

      else if (strcmp(name,"supported-assertions") == 0)
	    gen_assertions = "supported-assertions";

      else if (strcmp(name,"no-assertions") == 0)
	    gen_assertions = "no-assertions";

      else if (strcmp(name,"std-include") == 0)
	     gen_std_include = 1;

      else if (strcmp(name,"no-std-include") == 0)
	     gen_std_include = 0;

      else if (strcmp(name,"relative-include") == 0)
	    gen_relative_include = 1;

      else if (strcmp(name,"no-relative-include") == 0)
	    gen_relative_include = 0;

      else if (strcmp(name,"io-range-error") == 0)
	    gen_io_range_error = "io-range-error";

      else if (strcmp(name,"no-io-range-error") == 0)
	    gen_io_range_error = "no-io-range-error";

      else if (strcmp(name,"strict-ca-eval") == 0)
	    gen_strict_ca_eval = "strict-ca-eval";

      else if (strcmp(name,"no-strict-ca-eval") == 0)
	    gen_strict_ca_eval = "no-strict-ca-eval";

      else if (strcmp(name,"strict-expr-width") == 0)
	    gen_strict_expr_width = "strict-expr-width";

      else if (strcmp(name,"no-strict-expr-width") == 0)
	    gen_strict_expr_width = "no-strict-expr-width";

      else if (strcmp(name,"shared-loop-index") == 0)
	    gen_shared_loop_index = "shared-loop-index";

      else if (strcmp(name,"no-shared-loop-index") == 0)
	    gen_shared_loop_index = "no-shared-loop-index";

      else if (strcmp(name,"verilog-ams") == 0)
	    gen_verilog_ams = "verilog-ams";

      else if (strcmp(name,"no-verilog-ams") == 0)
	    gen_verilog_ams = "no-verilog-ams";

      else {
	    fprintf(stderr, "Unknown/Unsupported Language generation "
		    "%s\n\n", name);
	    fprintf(stderr, "Supported generations are:\n");
	    fprintf(stderr, "    1995    -- IEEE1364-1995\n"
		            "    2001    -- IEEE1364-2001\n"
		            "    2005    -- IEEE1364-2005\n"
		            "    2005-sv -- IEEE1800-2005\n"
		            "    2009    -- IEEE1800-2009\n"
		            "    2012    -- IEEE1800-2012\n"
		            "Other generation flags:\n"
		            "    assertions | supported-assertions | no-assertions\n"
		            "    specify | no-specify\n"
		            "    interconnect | no-interconnect\n"
		            "    verilog-ams | no-verilog-ams\n"
		            "    std-include | no-std-include\n"
		            "    relative-include | no-relative-include\n"
		            "    xtypes | no-xtypes\n"
		            "    icarus-misc | no-icarus-misc\n"
		            "    io-range-error | no-io-range-error\n"
		            "    strict-ca-eval | no-strict-ca-eval\n"
		            "    strict-expr-width | no-strict-expr-width\n"
		            "    shared-loop-index | no-shared-loop-index\n");

	    return 1;
      }

      return 0;
}

static int process_depfile(const char*name)
{
      const char*cp = strchr(name, '=');
      if (cp) {
            int match_length = (int)(cp - name) + 1;
            if (strncmp(name, "all=", match_length) == 0) {
                  depmode = 'a';
            } else if (strncmp(name, "include=", match_length) == 0) {
                  depmode = 'i';
            } else if (strncmp(name, "module=", match_length) == 0) {
                  depmode = 'm';
            } else if (strncmp(name, "prefix=", match_length) == 0) {
                  depmode = 'p';
            } else {
                  fprintf(stderr, "Unknown dependency file mode '%.*s'\n\n",
                          match_length - 1, name);
                  fprintf(stderr, "Supported modes are:\n");
                  fprintf(stderr, "    all\n");
                  fprintf(stderr, "    include\n");
                  fprintf(stderr, "    module\n");
                  fprintf(stderr, "    prefix\n");
                  return -1;
	    }
            depfile = cp + 1;
      } else {
            depmode = 'a';
            depfile = name;
      }
      return 0;
}

static void add_env_vpi_module_path(const char*path)
{
      env_vpi_path_list_size += 1;
      env_vpi_path_list = (const char**)realloc(env_vpi_path_list,
					  env_vpi_path_list_size*sizeof(char*));
      env_vpi_path_list[env_vpi_path_list_size-1] = path;
}

static void get_env_vpi_module_paths(void)
{
      char *var = getenv("IVERILOG_VPI_MODULE_PATH");
      char *ptr, *end;

      if (!var)
	    return;

      var = strdup(var);
#ifdef __MINGW32__
      convert_to_MS_path(var);
#endif
      ptr = var;
      end = var+strlen(var);
      int len = 0;
      while (ptr <= end) {
	    if (*ptr == 0 || *ptr == path_sep) {
		  *ptr = 0;
		  if (len > 0) {
		       add_env_vpi_module_path(var);
		  }
		  len = 0;
		  var = ptr+1;
	    } else {
		  len++;
	    }
	    ptr++;
      }
}

static void add_vpi_module_path(const char*path)
{
#ifdef __MINGW32__
      char*tmp_path = strdup(path);
      convert_to_MS_path(tmp_path);
      path = tmp_path;
#endif
      vpi_path_list_size += 1;
      vpi_path_list = (const char**)realloc(vpi_path_list,
				      vpi_path_list_size*sizeof(char*));
      vpi_path_list[vpi_path_list_size-1] = path;
}

static int probe_for_vpi_module(const char*base_path, const char*name,
				char*path, unsigned path_size)
{
      snprintf(path, path_size, "%s%c%s.vpi", base_path, sep, name);
      if (access(path, R_OK) == 0)
	    return 1;

      snprintf(path, path_size, "%s%c%s.vpl", base_path, sep, name);
      if (access(path, R_OK) == 0)
	    return 1;

      return 0;
}

/*
 * If it exists add the VPI file for the given module.
 */
static void add_vpi_file(const char *name)
{
      const char*base_dir = vpi_dir ? vpi_dir : base;

      char path[4096];

#ifdef __MINGW32__
      char*tmp_name = strdup(name);
      convert_to_MS_path(tmp_name);
      name = tmp_name;
#endif

      int found = 0;
      if (strchr(name, sep)) {
	      /* If the name has at least one directory character in it
		 then assume it is a complete name, maybe including any
		 possible .vpi or .vpl suffix. */
	    found = access(name, R_OK) == 0;
	    if (!found) {
		  snprintf(path, sizeof(path), "%s.vpi", name);
		  found = access(path, R_OK) == 0;
		  if (!found) {
			snprintf(path, sizeof(path), "%s.vpl", name);
			found = access(path, R_OK) == 0;
		  }
	    } else {
		  strncpy(path, name, sizeof(path) - 1);
	    }
      } else {
	    for (unsigned idx = 0; !found && (idx < vpi_path_list_size); idx += 1) {
		  found = probe_for_vpi_module(vpi_path_list[idx], name,
					       path, sizeof(path));
	    }
	    for (unsigned idx = 0; !found && (idx < env_vpi_path_list_size); idx += 1) {
		  found = probe_for_vpi_module(env_vpi_path_list[idx], name,
					       path, sizeof(path));
	    }
	    if (!found) {
		  found = probe_for_vpi_module(base_dir, name,
					       path, sizeof(path));
	    }
      }
      if (found) {
	    fprintf(iconfig_file, "module:%s\n", path);
      } else {
	    fprintf(stderr, "Unable to find VPI module '%s'\n", name);
      }
#ifdef __MINGW32__
      free(tmp_name);
#endif
}

static void find_ivl_root_failed(const char *reason)
{
      fprintf(stderr, "Cannot locate IVL modules : %s\n", reason);
      exit(1);
}

static void find_ivl_root(void)
{
#ifdef __MINGW32__
      const char *ivl_lib_prefix = "\\lib";
      const char *ivl_lib_suffix = "\\ivl" IVL_SUFFIX;
#else
      const char *ivl_lib_prefix = IVL_LIB;
      const char *ivl_lib_suffix = "/ivl" IVL_SUFFIX;
#endif
      ssize_t len = 0;
      char *s;

#ifndef __MINGW32__
	/* First try the location specified in the build process. */
      if (access(IVL_ROOT, F_OK) != -1) {
	    assert(strlen(IVL_ROOT) < sizeof ivl_root);
	    strcpy(ivl_root, IVL_ROOT);
	    return;
      }
#endif
	/* If that fails, calculate the ivl_root from the path to the
	   command. This is always necessary on Windows because of the
	   installation process, but may also be necessary on other OSs
	   if the package has been relocated.

	   On Windows we know the command path is formed like this:

		$(prefix)\bin\iverilog.exe

	   The module path in a Windows installation is the path:

		$(prefix)\lib\ivl$(suffix)

	   so we chop the file name and the last directory by
	   turning the last two \ characters to null. Then we append
	   the lib\ivl$(suffix) to finish.

	   On other OSs, we expect the command path to be:

		$(prefix)/bin/iverilog

	   and the module path to be:

		$(prefix)/$(lib)/ivl$(suffix)

	   so we extract the $(prefix) from the command location as for
	   Windows and the $(lib) from IVL_LIB. This will of course fail
	   if the user has overridden $(bindir) or $(libdir), but there's
	   not a lot we can do in that case.
 */
#if defined(__MINGW32__)
      char tmppath[MAXSIZE];
      len = GetModuleFileName(NULL, tmppath, sizeof tmppath);
      if (len >= (ssize_t) sizeof ivl_root) {
	    find_ivl_root_failed("command path exceeds size of string buffer.");
      }
	/* Convert to a short name to remove any embedded spaces. */
      len = GetShortPathName(tmppath, ivl_root, sizeof ivl_root);
#elif defined(__APPLE__)
	  uint32_t size = sizeof ivl_root;
	  len = _NSGetExecutablePath(ivl_root, &size) + 1;
#else
      len = readlink("/proc/self/exe", ivl_root, sizeof ivl_root);
#endif
      if (len >= (ssize_t) sizeof ivl_root) {
	    find_ivl_root_failed("command path exceeds size of string buffer.");
      }
      if (len <= 0) {
	      // We've failed, but we may yet find a -B option on the command line.
	      // Use the built-in path so the user sees a sensible error message.
	    assert(strlen(IVL_ROOT) < sizeof ivl_root);
	    strcpy(ivl_root, IVL_ROOT);
	    return;
      }
      s = strrchr(ivl_root, sep);
      if (s == 0) {
	    find_ivl_root_failed("missing first separator in command path.");
      }
      *s = 0;
      s = strrchr(ivl_root, sep);
      if (s == 0) {
	    find_ivl_root_failed("missing second separator in command path.");
      }
      *s = 0;
      len = s - ivl_root;
      s = strrchr(ivl_lib_prefix, sep);
      assert(s);
      if (len + strlen(s) + strlen(ivl_lib_suffix) >= (ssize_t) sizeof ivl_root) {
	    find_ivl_root_failed("module path exceeds size of string buffer.");
      }
      strcat(ivl_root, s);
      strcat(ivl_root, ivl_lib_suffix);
}

static void print_runtime_paths(void) {
	printf("includedir: %s\n", IVL_INCLUDE_INSTALL_DIR);
}

int main(int argc, char **argv)
{
      int e_flag = 0;
      int version_flag = 0;
      int opt;

      find_ivl_root();
      base = ivl_root;

      get_env_vpi_module_paths();

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

      while ((opt = getopt(argc, argv, "B:c:D:d:Ef:g:hl:I:iL:M:m:N:o:P:p:RSs:T:t:uvVW:y:Y:")) != EOF) {

	    switch (opt) {
		case 'B':
		    /* The individual components can be located by a
		       single base, or by individual bases. The first
		       character of the path indicates which path the
		       user is specifying. */
		  switch (optarg[0]) {
		      case 'M': /* Path for the VPI modules */
			vpi_dir = optarg+1;
			break;
		      case 'P': /* Path for the ivlpp preprocessor */
			ivlpp_dir = optarg+1;
			break;
		      case 'V': /* Path for the vhdlpp VHDL processor */
			vhdlpp_dir = optarg+1;
			break;
		      default: /* Otherwise, this is a default base. */
			base=optarg;
			break;
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
		case 'P':
		  defparm_size += 1;
		  defparm_base = (const char**)realloc(defparm_base,
		                                   defparm_size*sizeof(char*));
		  defparm_base[defparm_size-1] = optarg;
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

		case 'i':
		  ignore_missing_modules = 1;
		  break;

		case 'L':
		  add_vpi_module_path(optarg);
		  break;

		case 'l':
		  process_file_name(optarg, 1);
		  break;

		case 'M':
		  if (process_depfile(optarg) != 0)
                        return -1;
		  break;

		case 'm':
		  add_vpi_file(optarg);
		  break;

		case 'N':
		  npath = optarg;
		  break;

		case 'o':
		  opath = optarg;
		  break;

		case 'R':
		  print_runtime_paths();
		  return 0;

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
		case 'u':
		  separate_compilation_flag = 1;
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
		  fclose(source_file);
		  remove(source_path);
		  free(source_path);
		  fclose(defines_file);
		  remove(defines_path);
		  free(defines_path);
		  fclose(iconfig_file);
		  remove(iconfig_path);
		  free(iconfig_path);
		  remove(compiled_defines_path);
		  free(compiled_defines_path);
		  while( (command_filename = get_cmd_file()) ) {
			free(command_filename);
		  }
		  return 1;
	    }
      }

      if (strcmp(gen_verilog_ams,"verilog-ams") == 0)
	    fprintf(defines_file, "D:__VAMS_ENABLE__=1\n");
      if (synth_flag)
	    fprintf(defines_file, "D:__ICARUS_SYNTH__=1\n");

      if (vpi_dir == 0)
	    vpi_dir = base;
      if (ivlpp_dir == 0)
	    ivlpp_dir = base;
      if (vhdlpp_dir == 0)
	    vhdlpp_dir = base;

      if (version_flag || verbose_flag) {
	    printf("Icarus Verilog version " VERSION " (" VERSION_TAG ")\n\n");
	    printf("%s\n\n", COPYRIGHT);
	    puts(NOTICE);
      }

	/* Make a common conf file path to reflect the target. */
      snprintf(iconfig_common_path, sizeof iconfig_common_path, "%s%c%s%s.conf",
	      base, sep, targ, synth_flag? "-s" : "");

	/* Write values to the iconfig file. */
      fprintf(iconfig_file, "basedir:%s\n", base);

	/* Tell the core where to find the system VPI modules. */
      fprintf(iconfig_file, "module:%s%csystem.vpi\n", vpi_dir, sep);
      fprintf(iconfig_file, "module:%s%cvhdl_sys.vpi\n", vpi_dir, sep);
      fprintf(iconfig_file, "module:%s%cvhdl_textio.vpi\n", vpi_dir, sep);

	/* If verilog-2005/09/12 is enabled or icarus-misc or verilog-ams,
	 * then include the v2005_math library. */
      if (strcmp(generation, "2005") == 0 ||
          strcmp(generation, "2009") == 0 ||
          strcmp(generation, "2012") == 0 ||
          strcmp(gen_icarus, "icarus-misc") == 0 ||
          strcmp(gen_verilog_ams, "verilog-ams") == 0) {
	    fprintf(iconfig_file, "module:%s%cv2005_math.vpi\n", vpi_dir, sep);
      }
	/* If verilog-ams or icarus_misc is enabled, then include the
	 * va_math module as well. */
      if (strcmp(gen_verilog_ams,"verilog-ams") == 0 ||
          strcmp(gen_icarus, "icarus-misc") == 0) {
	    fprintf(iconfig_file, "module:%s%cva_math.vpi\n", vpi_dir, sep);
      }
      /* If verilog-2009 (SystemVerilog) is enabled, then include the
         v2009 module. */
      if (strcmp(generation, "2005-sv") == 0 ||
          strcmp(generation, "2009") == 0 ||
          strcmp(generation, "2012") == 0) {
	    fprintf(iconfig_file, "module:%s%cv2009.vpi\n", vpi_dir, sep);
      }

      if (mtm != 0) fprintf(iconfig_file, "-T:%s\n", mtm);
      fprintf(iconfig_file, "generation:%s\n", generation);
      fprintf(iconfig_file, "generation:%s\n", gen_specify);
      fprintf(iconfig_file, "generation:%s\n", gen_interconnect);
      fprintf(iconfig_file, "generation:%s\n", gen_assertions);
      fprintf(iconfig_file, "generation:%s\n", gen_xtypes);
      fprintf(iconfig_file, "generation:%s\n", gen_io_range_error);
      fprintf(iconfig_file, "generation:%s\n", gen_strict_ca_eval);
      fprintf(iconfig_file, "generation:%s\n", gen_strict_expr_width);
      fprintf(iconfig_file, "generation:%s\n", gen_shared_loop_index);
      fprintf(iconfig_file, "generation:%s\n", gen_verilog_ams);
      fprintf(iconfig_file, "generation:%s\n", gen_icarus);
      fprintf(iconfig_file, "warnings:%s\n", warning_flags);
      fprintf(iconfig_file, "ignore_missing_modules:%s\n", ignore_missing_modules ? "true" : "false");
      fprintf(iconfig_file, "out:%s\n", opath);
      if (depfile) {
            fprintf(iconfig_file, "depfile:%s\n", depfile);
            fprintf(iconfig_file, "depmode:%c\n", depmode);
      }

      while ( (command_filename = get_cmd_file()) ) {
	    int rc;

	    if (( fp = fopen(command_filename, "r")) == NULL ) {
		  fprintf(stderr, "%s: cannot open command file %s "
			  "for reading.\n", argv[0], command_filename);
		  return 1;
	    }

	    cfreset(fp, command_filename);
	    rc = cfparse();
	    if (rc != 0 || command_file_errors > 0) {
		  fprintf(stderr, "%s: parsing failed in base command "
		          "file %s.\n", argv[0], command_filename);
		  return 1;
	    }
            free(command_filename);
            fclose(fp);
      }
      destroy_lexor();

      if (depfile) {
	    fprintf(defines_file, "M%c:%s\n", depmode, depfile);
      }

      if (vhdlpp_work == 0)
	    vhdlpp_work = "ivl_vhdl_work";
      fprintf(defines_file, "vhdlpp:%s%cvhdlpp\n", vhdlpp_dir, sep);
      fprintf(defines_file, "vhdlpp-work:%s\n", vhdlpp_work);
      for (unsigned idx = 0 ; idx < vhdlpp_libdir_cnt ; idx += 1)
	    fprintf(defines_file, "vhdlpp-libdir:%s\n", vhdlpp_libdir[idx]);

    /* Process parameter definition from command line. The last
       defined would override previous ones. */
      int pitr;
      for (pitr = 0; pitr < defparm_size; pitr++)
        process_parameter(defparm_base[pitr]);
      free(defparm_base);
      defparm_base = 0;
      defparm_size = 0;

	/* Finally, process all the remaining words on the command
	   line as file names. */
      for (int idx = optind ;  idx < argc ;  idx += 1)
	    process_file_name(argv[idx], 0);

	/* If the use of a default include directory is not
	   specifically disabled, then write that directory as the
	   very last include directory to use... always. */
      if (gen_std_include) {
	    fprintf(defines_file, "I:%s%cinclude\n", base, sep);
      }

      if (gen_relative_include) {
	    fprintf(defines_file, "relative include:true\n");
      } else {
	    fprintf(defines_file, "relative include:false\n");
      }

      fclose(source_file);
      source_file = 0;

      fclose(defines_file);
      defines_file = 0;

	/* If we are planning on opening a dependencies file, then
	   open and truncate it here. The other phases of compilation
	   will append to the file, so this is necessary to make sure
	   it starts out empty. */
      if (depfile) {
            FILE *fd = fopen(depfile, "w");
            if (!fd) {
                  fprintf(stderr, "%s: can't open %s file.\n\n%s\n", argv[0], depfile, HELP);
                  return 1;
            }
            fclose(fd);
      }

      if (source_count == 0 && !version_flag) {
	    fprintf(stderr, "%s: no source files.\n\n%s\n", argv[0], HELP);
	    return 1;
      }

      fprintf(iconfig_file, "iwidth:%u\n", integer_width);

      fprintf(iconfig_file, "widthcap:%u\n", width_cap);

	/* Write the preprocessor command needed to preprocess a
	   single file. This may be used to preprocess library
	   files. */
      fprintf(iconfig_file, "ivlpp:%s%civlpp %s -L -F\"%s\" -P\"%s\"\n",
	      ivlpp_dir, sep,
              strchr(warning_flags, 'r') ? "-Wredef-all" :
              strchr(warning_flags, 'R') ? "-Wredef-chg" : "",
              defines_path, compiled_defines_path
      );

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
