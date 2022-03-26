/*
 * Copyright (c) 2015-2022 Martin Whitaker
 * Copyright (c) 2002 Gus Baldauf (gus@picturel.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 *	iverilog-vpi.c
 *
 *	this program provides the functionality of iverilog-vpi.sh under Windows
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <windows.h>

static void setup_ivl_environment(void);
static void assign(char **ptr, char *str);

/* The compile options: compiler, flags, etc. are in here */
#include "config.h"

/* pointers to global strings */

static struct global_strings {
	char *pCCSRC;		/* list of C source files (*.c) */
	char *pCXSRC;		/* list of C++ source files (*.cc, *.cpp) */
	char *pOBJ;		/* list of object files */
	char *pLIB;		/* list of library files */
	char *pINCS;		/* list of include directories */
	char *pDEFS;		/* list of definitions */
	char *pOUT;		/* output file name (.vpi extension), if 0 length then no source files specified */
	char *pMINGW;		/* path to MinGW directory */
	char *pIVL;		/* path to IVL directory */
	char *pCCFLAGS;		/* compiler flags for compiling C source files */
	char *pCXFLAGS;		/* compiler flags for compiling C++ source files */
	char *pLDLIBS;		/* linker flags for final linking stage */
	char *pCCNAME;		/* base name of compiler */
	char *pLD;		/* what to use for a linker */
} gstr;


static void deInitDynString(char *str)
{
	free(str);
}

/* when finished, free allocated memory and return error code */

static void myExit(int exitVal)
{
	deInitDynString(gstr.pCCSRC);
	deInitDynString(gstr.pCXSRC);
	deInitDynString(gstr.pOBJ);
	deInitDynString(gstr.pLIB);
	deInitDynString(gstr.pINCS);
	deInitDynString(gstr.pDEFS);
	deInitDynString(gstr.pOUT);
	deInitDynString(gstr.pMINGW);
	deInitDynString(gstr.pIVL);
	deInitDynString(gstr.pCCFLAGS);
	deInitDynString(gstr.pCXFLAGS);
	deInitDynString(gstr.pLDLIBS);
	deInitDynString(gstr.pCCNAME);
	deInitDynString(gstr.pLD);

	exit(exitVal);
}

/* display usage summary and exit */

static void usage(void)
{
	fprintf(stderr, "usage: iverilog-vpi" IVERILOG_SUFFIX " [options] [src and obj files]...\n");
	fprintf(stderr, "   or  iverilog-vpi" IVERILOG_SUFFIX " -mingw=dir\n");
	myExit(1);
}

static void initDynString(char **str)
{
	*str = (char *) malloc(1);

	if (!*str) {
		fprintf(stderr, "error: out of memory\n");
		myExit(4);
	}

	*str[0] = 0;
}

/* initialize dynamic memory buffers */

static void init(void)
{
	char *ptr;

	initDynString(&gstr.pCCSRC);
	initDynString(&gstr.pCXSRC);
	initDynString(&gstr.pOBJ);
	initDynString(&gstr.pLIB);
	initDynString(&gstr.pINCS);
	initDynString(&gstr.pDEFS);
	initDynString(&gstr.pOUT);
	initDynString(&gstr.pMINGW);
	initDynString(&gstr.pIVL);
	initDynString(&gstr.pCCFLAGS);
	initDynString(&gstr.pCXFLAGS);
	initDynString(&gstr.pLDLIBS);
	initDynString(&gstr.pCCNAME);
	initDynString(&gstr.pLD);

	  /* Get the base name of the C compiler. */
	assign(&gstr.pCCNAME, IVERILOG_VPI_CC);
	ptr = strchr(gstr.pCCNAME, ' ');
	if (ptr != NULL) *ptr = '\0';

	  /* By default use the C compiler to link the programs. */
	assign(&gstr.pLD, IVERILOG_VPI_CC);
}

/* return true if "str" is terminated with with "end", case insensitive */

static int endsIn (char *end, char *str)
{
	char *ext;

	if (strlen(end) >= strlen(str))
		return 0;

	ext = str + (strlen(str) - strlen(end));

	return stricmp(end, ext) ? 0 : 1;
}

/* return true if "str" begins with "prefix", case insensitive */

static int startsWith (char *prefix, char *str)
{
	if (strlen(prefix) >= strlen(str))
		return 0;

	return strnicmp(prefix, str, strlen(prefix)) ? 0 : 1;
}

/* append "app" to "ptr", allocating memory as needed    */
/*   if count is zero, then copy all characters of "app" */

static void appendn (char **ptr, char *app, size_t count)
{
	char *nptr = (char *) realloc(*ptr, strlen(*ptr) +
                                            (count ? count : strlen(app)) + 1);

	if (nptr == NULL) {
		fprintf(stderr, "error: out of memory\n");
		free(*ptr);
		myExit(4);
	}
	*ptr = nptr;

	if (count)
		strncat(*ptr, app, count);
	else
		strcat(*ptr, app);
}

/* append "app" to "ptr", allocating memory as needed    */

static void append (char **ptr, char *app)
{
	appendn(ptr, app, 0);
}

/* if the string does not end with a backslash, add one */

static void appendBackSlash(char **str)
{
	if ((*str)[strlen(*str)-1] != '\\')
		append(str, "\\");
}

/* copy count characters of "str" to "ptr", allocating memory as needed */
/*   if count is zero, then copy all characters of "str"                */

static void assignn (char **ptr, char *str, size_t count)
{
	char *nptr = (char *) realloc(*ptr, (count ? count : strlen(str)) + 1);

	if (nptr == NULL) {
		fprintf(stderr, "error: out of memory\n");
		free(*ptr);
		myExit(4);
	}
	*ptr = nptr;

	if (count) {
		strncpy(*ptr, str, count);
		(*ptr)[count] = 0;
	}
	else
		strcpy(*ptr, str);
}

/* copy count characters of "str" to "ptr", allocating memory as needed */

static void assign (char **ptr, char *str)
{
	assignn(ptr, str, 0);
}

/* get a copy of a Icarus Verilog registry string key */

static int GetRegistryKey(char *key, char **value)
{
	long lrv;
	HKEY hkKey;
	char *regKeyBuffer;
	DWORD regKeyType, regKeySize;

	lrv = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Icarus Verilog", 0, KEY_QUERY_VALUE, &hkKey);
	if (lrv != ERROR_SUCCESS)
		return 0;

	lrv = RegQueryValueEx(hkKey, key, NULL, &regKeyType, NULL, &regKeySize);
	if ((lrv != ERROR_SUCCESS) || (regKeyType != REG_SZ) || (!regKeySize)) {
		RegCloseKey(hkKey);
		return 0;
	}

	regKeyBuffer = (char *) malloc(regKeySize+1);
	if (!regKeyBuffer) {
		RegCloseKey(hkKey);
		fprintf(stderr, "error: out of memory\n");
		myExit(4);
	}
	regKeyBuffer[regKeySize] = 0;		/* makes sure there is a trailing NULL */

	/* This needs an unsigned char *, but for MinGW the char is signed. */
	lrv = RegQueryValueEx(hkKey, key, NULL, &regKeyType,
	                      (unsigned char *) regKeyBuffer, &regKeySize);
	if ((lrv != ERROR_SUCCESS) || (regKeyType != REG_SZ) || (!regKeySize)) {
		RegCloseKey(hkKey);
		free(regKeyBuffer);
		return 0;
	}

	RegCloseKey(hkKey);

	assign(value, regKeyBuffer);
	free(regKeyBuffer);

	return 1;
}

/* store a copy of a Icarus Verilog registry string key */

static void SetRegistryKey(char *key, char *value)
{
	long lrv;
	HKEY hkKey;
	DWORD res;

	lrv = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Software\\Icarus Verilog", 0, "",
			     REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkKey, &res);
	if (lrv != ERROR_SUCCESS) {
		char message[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lrv, LANG_USER_DEFAULT,
			      message, sizeof(message), NULL);
		fprintf(stderr, "error: couldn't write to registry - %s\n", message);
		if (lrv == ERROR_ACCESS_DENIED) {
			fprintf(stderr, "       try running as administrator\n");
		}
		return;
	}

	/* This needs an unsigned char *, but for MinGW the char is signed. */
	RegSetValueEx(hkKey, key, 0, REG_SZ, (unsigned char *) value,
	              strlen(value)+1);
	RegCloseKey(hkKey);

	printf("info:  storing %s in Windows' registry entry\n", value);
	printf("       HKEY_LOCAL_MACHINE\\Software\\Icarus Verilog\\%s\n", key);
}

/* parse the command line, assign results to global variable strings */

static int parse(int argc, char *argv[])
{
	int idx, srcFileCnt=0;

	char dot_c_ext[]    = ".c";
	char dot_cc_ext[]   = ".cc";
	char dot_cpp_ext[]  = ".cpp";
	char dot_o_ext[]    = ".o";
	char name_option[]  = "--name=";
	char lib_option[]   = "-l";
	char inc_option[]   = "-I";
	char mingw_option[] = "-mingw=";
	char def_option[]   = "-D";

	if (argc == 1) return 0;

	for (idx=1; idx<argc; ++idx) {
		  /* Check for C source files (*.c) */
		if (endsIn(dot_c_ext, argv[idx])) {
			++srcFileCnt;
			append(&gstr.pCCSRC, argv[idx]);
			append(&gstr.pCCSRC, " ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT, argv[idx],
			           strlen(argv[idx])-strlen(dot_c_ext));
		}
		  /* Check for C++ source files (*.cc) */
		else if (endsIn(dot_cc_ext, argv[idx])) {
			  /* We need to link with the C++ compiler. */
			assign(&gstr.pLD, IVERILOG_VPI_CXX);
			++srcFileCnt;
			append(&gstr.pCXSRC, argv[idx]);
			append(&gstr.pCXSRC, " ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT, argv[idx],
			           strlen(argv[idx])-strlen(dot_cc_ext));
		}
		  /* Check for C++ source files (*.cpp) */
		else if (endsIn(dot_cpp_ext, argv[idx])) {
			  /* We need to link with the C++ compiler. */
			assign(&gstr.pLD, IVERILOG_VPI_CXX);
			++srcFileCnt;
			append(&gstr.pCXSRC, argv[idx]);
			append(&gstr.pCXSRC, " ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT, argv[idx],
			           strlen(argv[idx])-strlen(dot_cpp_ext));
		}
		  /* Check for compiled object files */
		else if (endsIn(dot_o_ext, argv[idx])) {
			++srcFileCnt;
			append(&gstr.pOBJ, " ");
			append(&gstr.pOBJ, argv[idx]);
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT, argv[idx],
			           strlen(argv[idx])-strlen(dot_o_ext));
		}
		  /* Check for the -mingw option */
		else if (startsWith(mingw_option, argv[idx]))
			assignn(&gstr.pMINGW, argv[idx]+sizeof(mingw_option)-1,
			        strlen(argv[idx])-(sizeof(mingw_option)-1));
		  /* Check for the --name option */
		else if (startsWith(name_option, argv[idx])) {
			assignn(&gstr.pOUT, argv[idx]+sizeof(name_option)-1,
			        strlen(argv[idx])-(sizeof(name_option)-1));
		}
		  /* Check for the -l option */
		else if (startsWith(lib_option, argv[idx])) {
			append(&gstr.pLIB, " ");
			append(&gstr.pLIB, argv[idx]);
		}
		  /* Check for the -I option */
		else if (startsWith(inc_option, argv[idx])) {
			append(&gstr.pINCS, " ");
			append(&gstr.pINCS, argv[idx]);
		}
		  /* Check for the -D option */
		else if (startsWith(def_option, argv[idx])) {
			append(&gstr.pDEFS, " ");
			append(&gstr.pDEFS, argv[idx]);
		}
		  /* Check for the --cflags option */
		else if (stricmp("--cflags", argv[idx]) == 0) {
			setup_ivl_environment();
			printf("%s\n", gstr.pCCFLAGS);
			myExit(0);
		}
		  /* Check for the --ldflags option */
		else if (stricmp("--ldflags", argv[idx]) == 0) {
			printf("%s\n", IVERILOG_VPI_LDFLAGS);
			myExit(0);
		}
		  /* Check for the --ldlibs option */
		else if (stricmp("--ldlibs", argv[idx]) == 0) {
			setup_ivl_environment();
			printf("%s\n", gstr.pLDLIBS);
			myExit(0);
		}
		  /* Check for the --install-dir option */
		else if (stricmp("--install-dir", argv[idx]) == 0) {
			setup_ivl_environment();
			printf("%s\\lib\\ivl" IVERILOG_SUFFIX "\\.\n", gstr.pIVL);
			myExit(0);
		}
		  /* This is different than iverilog-vpi.sh, we don't
		   * ignore unknown arguments */
		else return 0;
	}

	  /* In case there is a --name without source/object files */
	if (0 == srcFileCnt) assign(&gstr.pOUT, "");

	if (*gstr.pOUT) {
		  /* We have a valid result file so add the .vpi extension */
		append(&gstr.pOUT, ".vpi");
	} else {
		  /* Unless we are setting the MinGW registry entry, it's
		     an error if there are no source or object files */
		if (!*gstr.pMINGW) return 0;
	}

	return 1;
}

/* do minimal check that the MinGW root directory looks valid */

static void checkMingwDir(char *root)
{
	int irv;
	struct _stat stat_buf;
	char *path;

	initDynString(&path);
	assign(&path, gstr.pMINGW);
	appendBackSlash(&path);
	append(&path, "bin\\");
        append(&path, gstr.pCCNAME);
	append(&path, ".exe");

	irv = _stat(path,&stat_buf);
	deInitDynString(path);

	if (irv) {
		fprintf(stderr, "error: %s\n", root);
		fprintf(stderr, "       does not appear to be the valid root directory of\n");
		fprintf(stderr, "       MinGW.  Use the -mingw option of iverilog-vpi.exe to\n");
		fprintf(stderr, "       point to the MinGW root directory.  For a Windows command\n");
		fprintf(stderr, "       shell the option would be something like -mingw=c:\\mingw\n");
		fprintf(stderr, "       For a Cygwin shell the option would be something like\n");
		fprintf(stderr, "       -mingw=c:\\\\mingw\n");
		myExit(5);
	}
}

/* see if we can find mingw root */

#define IVL_REGKEY_MINGW "MingwDir"

static void setup_mingw_environment(void)
{
	char buffer[1];		/* doesn't matter how big this is, as we don't use the result */
	char *path;

	if (*gstr.pMINGW) {
		checkMingwDir(gstr.pMINGW);
		if (!*gstr.pOUT) SetRegistryKey(IVL_REGKEY_MINGW, gstr.pMINGW);

	} else if (GetRegistryKey(IVL_REGKEY_MINGW, &gstr.pMINGW)) {
		checkMingwDir(gstr.pMINGW);

	} else if (SearchPath(NULL, gstr.pCCNAME, ".exe", sizeof(buffer), buffer, NULL)) {
		return;

	} else {
		fprintf(stderr, "error: cannot locate the MinGW C compiler - either add its location\n");
		fprintf(stderr, "       to the PATH environment variable or use the -mingw option of\n");
		fprintf(stderr, "       iverilog-vpi.exe to point to the MinGW root directory.  For\n");
		fprintf(stderr, "       a Windows command shell the option would be something like\n");
		fprintf(stderr, "       -mingw=c:\\mingw  For a Cygwin shell the option would be\n");
		fprintf(stderr, "       something like -mingw=c:\\\\mingw\n");
		myExit(5);
	}

	  /* Create new path with MinGW in it */
	initDynString(&path);
	assign(&path, "PATH=");
	append(&path, gstr.pMINGW);
	appendBackSlash(&path);
	append(&path, "bin;");
	append(&path, getenv("PATH"));

	  /* Place new path in environment */
	_putenv(path);
	deInitDynString(path);
}

/* find the iverilog root and initialise the compiler options */

static void setup_ivl_environment(void)
{
	char path[4096];
	char *ptr;

	  /* Extract the Icarus Verilog root directory from the path to the
	     command. The command path will look something like this:

		C:\iverilog\bin\iverilog-vpi.exe

	     The corresponding root directory is

		C:\iverilog

	     so we chop off the file name and the last directory. */
	GetModuleFileName(NULL, path, sizeof(path));
	ptr = strrchr(path, '\\');
	if (!ptr) {
		fprintf(stderr, "error: couldn't find start of program name in command path '%s'\n", path);
		myExit(6);
	}
	*ptr = 0;
	ptr = strrchr(path, '\\');
	if (!ptr) {
		fprintf(stderr, "error: couldn't find start of bin directory in command path '%s'\n", path);
		myExit(6);
	}
	*ptr = 0;
	assign(&gstr.pIVL, path);

	  /* Build up the CCFLAGS option string */
	assign(&gstr.pCCFLAGS, IVERILOG_VPI_CFLAGS " -I\"");
	append(&gstr.pCCFLAGS, gstr.pIVL);
	appendBackSlash(&gstr.pCCFLAGS);
	append(&gstr.pCCFLAGS, "include\\iverilog\"" IVERILOG_SUFFIX);

	  /* Build up the CXFLAGS option string */
	assign(&gstr.pCXFLAGS, IVERILOG_VPI_CXXFLAGS " -I\"");
	append(&gstr.pCXFLAGS, gstr.pIVL);
	appendBackSlash(&gstr.pCXFLAGS);
	append(&gstr.pCXFLAGS, "include\\iverilog\"" IVERILOG_SUFFIX);

	  /* Build up the LDFLAGS option string */
	assign(&gstr.pLDLIBS, "-L\"");
	append(&gstr.pLDLIBS, gstr.pIVL);
	appendBackSlash(&gstr.pLDLIBS);
	append(&gstr.pLDLIBS, "lib\" " IVERILOG_VPI_LDLIBS);
}

/* compile source modules */

static void compile(char *pSource, char *pFlags, char **pObject, int *compile_errors, char *compiler)
{
	char *ptr1 = pSource;
	char *ptr2 = strchr(ptr1, ' ');
	char *buf=0, *src=0, *obj=0;

	while (ptr2) {
		int len = ptr2 - ptr1;
		char *ostart;
		int olen;
		assignn(&src, ptr1, len);

		  /* Build the object file name */
		ostart = strrchr(src, '/');
		if (ostart == NULL) ostart = src;
		else ostart += 1;
		olen = strrchr(ostart, '.') - ostart;
		assignn(&obj, ostart, olen);
		append(&obj, ".o");

		  /* Build the compile line */
		assign(&buf, compiler);
		append(&buf, " -c -o ");
		append(&buf, obj);
		append(&buf, " ");
		append(&buf, gstr.pDEFS);
		append(&buf, " ");
		append(&buf, pFlags);
		append(&buf, " ");
		append(&buf, gstr.pINCS);
		append(&buf, " ");
		append(&buf, src);

		append (pObject, " ");
		append (pObject, obj);

		printf("Compiling %s...\n", src);

		if (system(buf)) ++*compile_errors;

		  /* advance to next token */
		ptr1 = ptr2 + 1;
		ptr2 = strchr(ptr1, ' ');
	}

	free(buf);
	free(src);
	free(obj);
}

/* using the global strings, compile and link */

static void compile_and_link(void)
{
	char *buf=0;
	int iRet, compile_errors = 0;

	  /* To make the output match iverilog-vpi.sh do not print out the
	   * root directories */
//	printf("MinGW root directory:  %s.\n", gstr.pMINGW);

	  /* compile the C source files (*.c) */
	compile(gstr.pCCSRC, gstr.pCCFLAGS, &gstr.pOBJ, &compile_errors, IVERILOG_VPI_CC );
	  /* compile the C++ source files (*.cc, *.cpp) */
	compile(gstr.pCXSRC, gstr.pCXFLAGS, &gstr.pOBJ, &compile_errors, IVERILOG_VPI_CXX);

	if (compile_errors) {
		fprintf(stderr, "iverilog-vpi: %d file(s) failed to compile.\n",
		        compile_errors);
		myExit(2);
	}

	  /* link */
	assign(&buf, gstr.pLD);
	append(&buf, " -o ");
	append(&buf, gstr.pOUT);
	append(&buf, " ");
	append(&buf, IVERILOG_VPI_LDFLAGS);
	append(&buf, " ");
	append(&buf, gstr.pOBJ);
	append(&buf, " ");
	append(&buf, gstr.pLIB);
	append(&buf, " ");
	append(&buf, gstr.pLDLIBS);

	printf("Making %s from %s...\n", gstr.pOUT,gstr.pOBJ);

	iRet = system(buf);
	free(buf);
	if (iRet) myExit(3);
}

/* program execution starts here */

int main(int argc, char *argv[])
{
	init();

	if (!parse(argc, argv)) usage();

	setup_mingw_environment();
	setup_ivl_environment();

	  /* are there any source or object files specified */
	if (*gstr.pOUT) compile_and_link();

	myExit(0);
	return 0; // eliminate warnings.
}
