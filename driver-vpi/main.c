/*
 *	iverilog-vpi.c
 *
 *	this program is provides the functionality
 *	of iverilog-vpi.sh under Win32
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

/* Macros used for compiling and linking */

#define IVERILOG_VPI_CC        "gcc"
#define IVERILOG_VPI_CXX       "gcc"
#define IVERILOG_VPI_CFLAGS    "-O"					/* -I appended  later */
#define IVERILOG_VPI_LD        "gcc"
#define IVERILOG_VPI_LDFLAGS   "-shared -Wl,--enable-auto-image-base"
#define IVERILOG_VPI_LDLIBS    "-lveriuser -lvpi"	/* -L prepended later */

/* pointers to global strings */

static struct global_strings {
	char *pCCSRC;		// list of C source files
	char *pCXSRC;		// list of C++ source files
	char *pOBJ;			// list of object files
	char *pLIB;			// list of library files
	char *pOUT;			// output file name (.vpi extension)
	char *pMINGW;		// path to MinGW directory
	char *pIVL;			// path to IVL directory
	char *pCFLAGS;		// CFLAGS option
	char *pLDLIBS;		// LDLIBS option
	char *pNewPath;		// new PATH environment variable setting
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

static void usage(char *name)
{
	fprintf(stderr,"usage: %s [--name=name] [-llibrary] [-mingw=dir] [-ivl=dir] sourcefile...\n",name);
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
	regKeyBuffer[regKeySize] = 0;		// makes sure there is a trailing NULL

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
}

/* parse the command line, assign results to global variable strings */

static int parse(int argc, char *argv[])
{
	int idx;

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
			append(&gstr.pCCSRC,argv[idx]);
			append(&gstr.pCCSRC," ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT,argv[idx],strlen(argv[idx])-strlen(dot_c_ext));
		}
		else if (endsIn(dot_cc_ext,argv[idx])) {		/* check for C++ source files */
			append(&gstr.pCXSRC,argv[idx]);
			append(&gstr.pCXSRC," ");
			if (!*gstr.pOUT)
			   assignn(&gstr.pOUT,argv[idx],strlen(argv[idx])-strlen(dot_cc_ext));
		}
		else if (endsIn(dot_o_ext,argv[idx])) {			/* check for compiled object files */
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

	if (!*gstr.pOUT)
		usage(argv[0]);

	append(&gstr.pOUT,".vpi");		/* the result file should have a .vpi extension */
	append(&gstr.pOUT," ");

	return 1;
}

/* see if we can find iverilog and mingw */

#define IVL_REGKEY_MINGW "MingwDir"

static void setup_mingw_environment()
{
	char *pOldPATH;
	
	char *RegKeyMinGW;
	initDynString(&RegKeyMinGW);

	if (*gstr.pMINGW)
		SetRegistryKey(IVL_REGKEY_MINGW,gstr.pMINGW);
	else
		if (!GetRegistryKey(IVL_REGKEY_MINGW,&RegKeyMinGW)) {
			fprintf(stderr,"error: can not locate MinGW directory, use the -mingw option\n");
			deInitDynString(RegKeyMinGW);
			myExit(5);
		}

	pOldPATH = getenv("PATH");					/* get current path */

	assign(&gstr.pNewPath,"PATH=");			/* create new path */
	append(&gstr.pNewPath,*gstr.pMINGW ? gstr.pMINGW : RegKeyMinGW);
	appendBackSlash(&gstr.pNewPath);
	append(&gstr.pNewPath,"bin;");
	append(&gstr.pNewPath,pOldPATH);

	_putenv(gstr.pNewPath);					/* place new path in environment variable */

	deInitDynString(RegKeyMinGW);
}

#define IVL_REGKEY_IVL   "InstallDir"

static void setup_ivl_environment()
{
	char *RegKeyIVL;
	initDynString(&RegKeyIVL);

	if (*gstr.pIVL)
		SetRegistryKey(IVL_REGKEY_IVL,gstr.pIVL);
	else
		if (!GetRegistryKey(IVL_REGKEY_IVL,&RegKeyIVL)) {
			fprintf(stderr,"error: can not locate Icarus Verilog directory, use the -ivl option\n");
			deInitDynString(RegKeyIVL);
			myExit(6);
		}

	/* build up the CFLAGS option string */

	assign(&gstr.pCFLAGS,IVERILOG_VPI_CFLAGS);
	append(&gstr.pCFLAGS," -I");
	append(&gstr.pCFLAGS,*gstr.pIVL ? gstr.pIVL : RegKeyIVL);
	appendBackSlash(&gstr.pCFLAGS);
	append(&gstr.pCFLAGS,"include");

	/* build up the LDFLAGS option string */

	assign(&gstr.pLDLIBS,"-L");
	append(&gstr.pLDLIBS,*gstr.pIVL ? gstr.pIVL : RegKeyIVL);
	appendBackSlash(&gstr.pLDLIBS);
	append(&gstr.pLDLIBS,"lib ");
	append(&gstr.pLDLIBS,IVERILOG_VPI_LDLIBS);

	deInitDynString(RegKeyIVL);
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
		usage(argv[0]);

	setup_mingw_environment();
	setup_ivl_environment();

	compile_and_link();

	myExit(0);
}
