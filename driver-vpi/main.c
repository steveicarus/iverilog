/*
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/*
 *	iverilog-vpi.c
 *
 *	this program provides the functionality of iverilog-vpi.sh under Win32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <windows.h>

/* Macros used for compiling and linking */

#define IVERILOG_VPI_CC        "gcc"				/* no .exe extension  */
#define IVERILOG_VPI_CXX       "gcc"				/* no .exe extension  */
#define IVERILOG_VPI_CFLAGS    "-O"					/* -I appended  later */
#define IVERILOG_VPI_LD        "gcc"				/* no .exe extension  */
#define IVERILOG_VPI_LDFLAGS   "-shared -Wl,--enable-auto-image-base"
#define IVERILOG_VPI_LDLIBS    "-lveriuser -lvpi"	/* -L prepended later */

/* pointers to global strings */

static struct global_strings {
	char *pCCSRC;		/* list of C source files */
	char *pCXSRC;		/* list of C++ source files */
	char *pOBJ;			/* list of object files */
	char *pLIB;			/* list of library files */
	char *pOUT;			/* output file name (.vpi extension), if 0 length then no source files specified */
	char *pMINGW;		/* path to MinGW directory */
	char *pIVL;			/* path to IVL directory */
	char *pCFLAGS;		/* CFLAGS option */
	char *pLDLIBS;		/* LDLIBS option */
	char *pNewPath;		/* new PATH environment variable setting */
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
	deInitDynString(gstr.pOUT);
	deInitDynString(gstr.pMINGW);
	deInitDynString(gstr.pIVL);
	deInitDynString(gstr.pCFLAGS);
	deInitDynString(gstr.pLDLIBS);
	deInitDynString(gstr.pNewPath);

	exit(exitVal);
}

/* display usage summary and exit */

static void usage()
{
	fprintf(stderr,"usage: iverilog-vpi [--name=name] [-llibrary] [-mingw=dir] [-ivl=dir] sourcefile...\n");
	fprintf(stderr,"   or  iverilog-vpi -mingw=dir\n");
	fprintf(stderr,"   or  iverilog-vpi -ivl=dir\n");
	myExit(1);
}

static void initDynString(char **str)
{
	*str = (char *) malloc(1);

	if (!*str) {
		fprintf(stderr,"error: out of memory\n");
		myExit(4);
	}

	*str[0] = 0;
}

/* initialize dynamic memory buffers */

static void init()
{
	initDynString(&gstr.pCCSRC);
	initDynString(&gstr.pCXSRC);
	initDynString(&gstr.pOBJ);
	initDynString(&gstr.pLIB);
	initDynString(&gstr.pOUT);
	initDynString(&gstr.pMINGW);
	initDynString(&gstr.pIVL);
	initDynString(&gstr.pCFLAGS);
	initDynString(&gstr.pLDLIBS);
	initDynString(&gstr.pNewPath);
}

/* return true if "str" is terminated with with "end", case insensitive */

static int endsIn (char *end, char *str)
{
	char *ext;

	if (strlen(end) >= strlen(str))
		return 0;

	ext = str + (strlen(str) - strlen(end));

	return stricmp(end,ext) ? 0 : 1;
}

/* return true if "str" begins with "prefix", case insensitive */

static int startsWith (char *prefix, char *str)
{
	if (strlen(prefix) >= strlen(str))
		return 0;

	return strnicmp(prefix,str,strlen(prefix)) ? 0 : 1;
}

/* append "app" to "ptr", allocating memory as needed    */
/*   if count is zero, then copy all characters of "app" */

static void appendn (char **ptr, char *app, int count)
{
	*ptr = (char *) realloc(*ptr,strlen(*ptr)+(count?count:strlen(app))+1);

	if (*ptr == NULL) {
		fprintf(stderr,"error: out of memory\n");
		myExit(4);
	}

	if (count)
		strncat(*ptr,app,count);
	else
		strcat(*ptr,app);
}

/* append "app" to "ptr", allocating memory as needed    */

static void append (char **ptr, char *app)
{
	appendn(ptr,app,0);
}

/* if the string does not end with a backslash, add one */

static void appendBackSlash(char **str)
{
	if ((*str)[strlen(*str)-1] != '\\')
		append(str,"\\");
}

/* copy count characters of "str" to "ptr", allocating memory as needed */
/*   if count is zero, then copy all characters of "str"                */

static void assignn (char **ptr, char *str, int count)
{
	*ptr = (char *) realloc(*ptr,(count?count:strlen(str))+1);

	if (*ptr == NULL) {
		fprintf(stderr,"error: out of memory\n");
		myExit(4);
	}

	if (count) {
		strncpy(*ptr,str,count);
		(*ptr)[count] = 0;
	}
	else
		strcpy(*ptr,str);
}

/* copy count characters of "str" to "ptr", allocating memory as needed */

static void assign (char **ptr, char *str)
{
	assignn(ptr,str,0);
}

/* get a copy of a Icarus Verilog registry string key */

static int GetRegistryKey(char *key, char **value)
{
	long lrv;
	HKEY hkKey;
	char *regKeyBuffer;
	DWORD regKeyType, regKeySize;

	lrv = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\Icarus Verilog",0,KEY_QUERY_VALUE,&hkKey);
	if (lrv != ERROR_SUCCESS)
		return 0;

	lrv = RegQueryValueEx(hkKey,key,NULL,&regKeyType,NULL,&regKeySize);
	if ((lrv != ERROR_SUCCESS) || (regKeyType != REG_SZ) || (!regKeySize)) {
		lrv = RegCloseKey(hkKey);
		return 0;
	}

	regKeyBuffer = (char *) malloc(regKeySize+1);
	if (!regKeyBuffer) {
		lrv = RegCloseKey(hkKey);
		fprintf(stderr,"error: out of memory\n");
		myExit(4);
	}
	regKeyBuffer[regKeySize] = 0;		/* makes sure there is a trailing NULL */

	lrv = RegQueryValueEx(hkKey,key,NULL,&regKeyType,regKeyBuffer,&regKeySize);
	if ((lrv != ERROR_SUCCESS) || (regKeyType != REG_SZ) || (!regKeySize)) {
		lrv = RegCloseKey(hkKey);
		free(regKeyBuffer);
		return 0;
	}

	RegCloseKey(hkKey);

	assign(value,regKeyBuffer);
	free(regKeyBuffer);

	return 1;
}

/* store a copy of a Icarus Verilog registry string key */

static void SetRegistryKey(char *key, char *value)
{
	HKEY hkKey;
	DWORD res;

	if (RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		"Software\\Icarus Verilog",
		0,
		"",
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,NULL,
		&hkKey,
		&res) != ERROR_SUCCESS)
			return;

	RegSetValueEx(hkKey,key,0,REG_SZ,value,strlen(value)+1);
	RegCloseKey(hkKey);

	printf("info:  storing %s in Windows' registry entry\n",value);
	printf("       HKEY_LOCAL_MACHINE\\Software\\Icarus Verilog\\%s\n",key);
}

/* parse the command line, assign results to global variable strings */

static int parse(int argc, char *argv[])
{
	int idx, srcFileCnt=0;

	char dot_c_ext[]    = ".c";
	char dot_cc_ext[]   = ".cc";
	char dot_o_ext[]    = ".o";
	char name_option[]  = "--name=";
	char lib_option[]   = "-l";
	char mingw_option[] = "-mingw=";
	char ivl_option[]   = "-ivl=";

	if (argc == 1)
		return 0;

	for (idx=1; idx<argc; ++idx) {
		if (endsIn(dot_c_ext,argv[idx])) {				/* check for C source files */
			++srcFileCnt;
			append(&gstr.pCCSRC,argv[idx]);
			append(&gstr.pCCSRC," ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT,argv[idx],strlen(argv[idx])-strlen(dot_c_ext));
		}
		else if (endsIn(dot_cc_ext,argv[idx])) {		/* check for C++ source files */
			++srcFileCnt;
			append(&gstr.pCXSRC,argv[idx]);
			append(&gstr.pCXSRC," ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT,argv[idx],strlen(argv[idx])-strlen(dot_cc_ext));
		}
		else if (endsIn(dot_o_ext,argv[idx])) {			/* check for compiled object files */
			++srcFileCnt;
			append(&gstr.pOBJ,argv[idx]);
			append(&gstr.pOBJ," ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT,argv[idx],strlen(argv[idx])-strlen(dot_o_ext));
		}
		else if (startsWith(name_option,argv[idx])) {	/* check for --name option */
			assignn(&gstr.pOUT,argv[idx]+sizeof(name_option)-1,strlen(argv[idx])-(sizeof(name_option)-1));
		}
		else if (startsWith(lib_option,argv[idx])) {	/* check for -l option */
			append(&gstr.pLIB,argv[idx]);
			append(&gstr.pLIB," ");
		}
		else if (startsWith(mingw_option,argv[idx]))	/* check for -mingw option */
			assignn(&gstr.pMINGW,argv[idx]+sizeof(mingw_option)-1,strlen(argv[idx])-(sizeof(mingw_option)-1));
		else if (startsWith(ivl_option,argv[idx]))		/* check for -ivl   option */
			assignn(&gstr.pIVL,argv[idx]+sizeof(ivl_option)-1,strlen(argv[idx])-(sizeof(ivl_option)-1));
		else
			return 0;	/* different from iverilog-vpi.sh, we don't ignore accept arguments */
	}

	if (0 == srcFileCnt)
		assign(&gstr.pOUT,"");				/* in case they used --name with no source files */

	if (!*gstr.pOUT) {						/* normally it's an error if there are no *.c,*.cc,*.o files */
		if (!*gstr.pMINGW && !*gstr.pIVL)	/* unless they are just setting the IVL or MinGW registry entries */
			usage();
	}
	else {
		append(&gstr.pOUT,".vpi");			/* the result file should have a .vpi extension */
		append(&gstr.pOUT," ");
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
	assign(&path,gstr.pMINGW);
	appendBackSlash(&path);
	append(&path,"bin\\" IVERILOG_VPI_CC ".exe");

	irv = _stat(path,&stat_buf);
	deInitDynString(path);

	if (irv) {
		fprintf(stderr,"error: %s does not appear to be the valid root directory\n",root);
		fprintf(stderr,"       of MinGW.  Use the -mingw option of iverilog-vpi.exe to\n");
		fprintf(stderr,"       point to the MinGW root directory.  For a Windows command\n");
		fprintf(stderr,"       shell the option would be something like -mingw=c:\\mingw\n");
		fprintf(stderr,"       For a Cygwin shell the option would be something like\n");
		fprintf(stderr,"       -mingw=c:\\\\mingw\n");
		myExit(5);
	}
}

/* do minimal check that the Icarus Verilog root directory looks valid */

static void checkIvlDir(char *root)
{
	int irv;
	struct _stat stat_buf;

	char *path;
	initDynString(&path);
	assign(&path,gstr.pIVL);
	appendBackSlash(&path);
	append(&path,"bin\\vvp.exe");

	irv = _stat(path,&stat_buf);
	deInitDynString(path);

	if (irv) {
		fprintf(stderr,"error: %s does not appear to be the valid root directory of\n",root);
		fprintf(stderr,"       Icarus Verilog.  Use the -ivl option of iverilog-vpi.exe to\n");
		fprintf(stderr,"       point to the Icarus Verilog root directory.  For a Windows\n");
		fprintf(stderr,"       command shell the option would be something like -ivl=c:\\iverilog\n");
		fprintf(stderr,"       For a Cygwin shell the option would be something like\n");
		fprintf(stderr,"       -ivl=c:\\\\iverilog\n");
		myExit(6);
	}
}

/* see if we can find mingw root */

#define IVL_REGKEY_MINGW "MingwDir"

static void setup_mingw_environment()
{
	char *pOldPATH = getenv("PATH");		/* get current path */

	if (*gstr.pMINGW) {
		checkMingwDir(gstr.pMINGW);
		SetRegistryKey(IVL_REGKEY_MINGW,gstr.pMINGW);
	}
	else
		if (!GetRegistryKey(IVL_REGKEY_MINGW,&gstr.pMINGW)) {
			fprintf(stderr,"error: can not locate the MinGW root directory, use the -mingw option of\n");
			fprintf(stderr,"       iverilog-vpi.exe to point to the MinGW root directory.  For\n");
			fprintf(stderr,"       a Windows command shell the option would be something like\n");
			fprintf(stderr,"       -mingw=c:\\mingw  For a Cygwin shell the option would be\n");
			fprintf(stderr,"       something like -mingw=c:\\\\mingw\n");
			myExit(5);
		}

	assign(&gstr.pNewPath,"PATH=");			/* create new path */
	append(&gstr.pNewPath,gstr.pMINGW);
	appendBackSlash(&gstr.pNewPath);
	append(&gstr.pNewPath,"bin;");
	append(&gstr.pNewPath,pOldPATH);

	_putenv(gstr.pNewPath);					/* place new path in environment variable */
}

/* see if we can find iverilog root */

#define IVL_REGKEY_IVL   "InstallDir"

static void setup_ivl_environment()
{
	if (*gstr.pIVL) {
		checkIvlDir(gstr.pIVL);
		SetRegistryKey(IVL_REGKEY_IVL,gstr.pIVL);
	}
	else
		if (!GetRegistryKey(IVL_REGKEY_IVL,&gstr.pIVL)) {
			fprintf(stderr,"error: can not locate the Icarus Verilog root directory, use the -ivl option\n");
			fprintf(stderr,"       of iverilog-vpi.exe to point to the Icarus Verilog root directory.\n");
			fprintf(stderr,"       For a Windows command shell the option would be something like\n");
			fprintf(stderr,"       -ivl=c:\\iverilog  For a Cygwin shell the option would be something\n");
			fprintf(stderr,"       like -ivl=c:\\\\iverilog\n");
			myExit(6);
		}

	/* build up the CFLAGS option string */

	assign(&gstr.pCFLAGS,IVERILOG_VPI_CFLAGS);
	append(&gstr.pCFLAGS," -I");
	append(&gstr.pCFLAGS,gstr.pIVL);
	appendBackSlash(&gstr.pCFLAGS);
	append(&gstr.pCFLAGS,"include");

	/* build up the LDFLAGS option string */

	assign(&gstr.pLDLIBS,"-L");
	append(&gstr.pLDLIBS,gstr.pIVL);
	appendBackSlash(&gstr.pLDLIBS);
	append(&gstr.pLDLIBS,"lib ");
	append(&gstr.pLDLIBS,IVERILOG_VPI_LDLIBS);
}

/* compile source modules */

static void compile(char *pSource, char **pObject, char *ext, int *compile_errors, char *compiler)
{
	char *ptr1 = pSource;
	char *ptr2 = strchr(pSource,' ');
	char *buf=0,*src=0,*obj=0;

	while (ptr2) {
		int len = ptr2 - ptr1;
		assignn(&src,ptr1,len);

		assignn(&obj,ptr1,len-strlen(ext));		/* strip off the extension */
		append (&obj,".o");

		assign (&buf,compiler);
		append (&buf," -c -o ");
		append (&buf,obj);
		append (&buf," ");
		append (&buf,gstr.pCFLAGS);
		append (&buf," ");
		append (&buf,src);

		append (pObject,obj);
		append (pObject," ");

		printf("%s\n",buf);

		if (system(buf))
			++*compile_errors;

		ptr1 = ptr2 + 1;					/* advance to next token */
		ptr2 = strchr(ptr1,' ');
	}

	free(buf);
	free(src);
	free(obj);
}

/* using the global strings, compile and link */

static void compile_and_link()
{
	char *buf=0;
	int iRet, compile_errors = 0;

	/* print out the mingw and ivl directories to help the user debug problems */

	printf("info:  %s will be used as the MinGW root directory.\n",gstr.pMINGW);
	checkMingwDir(gstr.pMINGW);

	printf("info:  %s will be used as the Icarus Verilog root directory.\n",gstr.pIVL);
	checkIvlDir(gstr.pIVL);

	/* compile */

	compile(gstr.pCCSRC,&gstr.pOBJ,".c" ,&compile_errors,IVERILOG_VPI_CC );		/* compile the C   source files */
	compile(gstr.pCXSRC,&gstr.pOBJ,".cc",&compile_errors,IVERILOG_VPI_CXX);		/* compile the C++ source files */

	if (compile_errors) {
		fprintf(stderr,"iverilog-vpi: Some %d files failed to compile.\n",compile_errors);
		myExit(2);
	}

	/* link */

	assign(&buf,IVERILOG_VPI_LD);
	append(&buf," -o ");
	append(&buf,gstr.pOUT);				/* has a trailing space */
	append(&buf,IVERILOG_VPI_LDFLAGS);
	append(&buf," ");
	append(&buf,gstr.pOBJ)				/* has a trailing space */;
	append(&buf,gstr.pLIB);				/* has a trailing space */
	append(&buf,gstr.pLDLIBS);

	printf("%s\n",buf);

	iRet = system(buf);

	free(buf);

	if (iRet)
		myExit(3);
}

/* program execution starts here */

int main(int argc, char *argv[])
{
	init();

	if (!parse(argc,argv))
		usage();

	setup_mingw_environment();
	setup_ivl_environment();

	if (*gstr.pOUT)				/* are there any *.c,*.cc,*.o files specified */
		compile_and_link();

	myExit(0);
	return 0; // eliminate warnings.
}
