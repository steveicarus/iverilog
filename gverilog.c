/*
 * gvlog.c - A driver for verilog compiler ivl
 * Copyright (C) 1999 Stefan Petersen (spe@geda.seul.org)

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#ifndef LIBDIR
# define LIBDIR "/usr/local/lib"
#endif
#ifndef INCLUDEDIR
# define INCLUDEDIR "/usr/local/include"
#endif

const char VERSION[] = "$Name:  $ $State: Exp $";


#define P_IF_SET(var) var.set ? var.name : ""

/* Div compiler settings */
struct compset {
  char compsw[5];
  char name[50];
  int set;
};

/* cpp include path */
struct compset cppincdir = {"-I", INCLUDEDIR, 1};

/* cpp library path */
struct compset cpplibdir ={"-L", LIBDIR, 1};

/* VPI module path */
struct compset VPImodpath = {"-f", LIBDIR "/ivl", 1};

/* ivl include path */
struct compset ivlppincdir = {"-I", "", 0};

/* ivl defines */
struct compset ivlppdefines = {"-D", "", 0};

/* ivl target setup */ 
/* Compilation target, default vvm */
typedef enum {VVM, XNF, OTHER} targettype;
struct target {
  targettype target;
  struct compset targetinfo;
};
struct target target  = {VVM, {"-t", "vvm", 0}};

/* output file */
struct compset outputfile = {"-o", "", 0};

/* topmodule name */
struct compset topmodule = {"-s", "", 0};

/* verilog files */
char verilogfiles[50] = "";


/* Temp files for storage */
char tmpPPfile[50];
char tmpCCfile[50];

/* Compilation flag info */
struct compileinfo {
  int pform;
  int execute;
  int elabnetlist;
  int debug;
};
struct compileinfo compileinfo = {0, 0, 0, 0};


void sorry(char optelem)
{
  printf("Sorry, switch -%c not implemented yet!\n", optelem);
}


targettype resolvtarget(char *target)
{
  if (strcmp(target, "vvm") == 0)
    return VVM;
  else if (strcmp(target, "xnf") == 0)
    return XNF;
  else 
    return OTHER;
}


void error_exit(void)
{
  fprintf(stderr, "verilog [-Dmacro[=defn]] [-Iincludepath] [-X] \n");
  fprintf(stderr, "[-x] [-o outputfilename] [-s topmodule] sourcefile[s]\n");
  exit(1);
}


void create_outputfilename(char *input, char *output)
{
  int dotpos=0;
  int spacepos=0;
  int i=0;

  for(i=strlen(input); i!=0; i--){
    if ((dotpos == 0) && (input[i] == '.')) dotpos = i;
    if ((spacepos == 0) && (input[i] == ' ')) spacepos = i;
  }
  
  if (dotpos == 0) {
    dotpos = strlen(input);
  }

  output = strncpy(output, &input[spacepos+1], dotpos-spacepos-1);

  return;
}


void preprocess(void)
{
  char *argument;

  argument = (char *)malloc(256);

  sprintf(argument, LIBDIR "/ivl/ivlpp %s %s -L -o%s %s",
	  P_IF_SET(ivlppdefines),
	  P_IF_SET(ivlppincdir),
	  tmpPPfile,
	  verilogfiles);
  if (compileinfo.debug) {
    printf("Executing command : \n%s\n", argument);
  } else {
    if (system(argument)) {
      fprintf(stderr, "Preprocessing failed. Terminating compilation\n");
      unlink(tmpPPfile);
      free(argument);
      exit(1);
    }
  }
  
  free(argument);
  return;
}


void compile(void)
{
  char *argument;

  argument = (char *)malloc(256);

  /* VPI_MODULE_PATH should be better integrated. */
  /* HACK */
  sprintf(argument, LIBDIR "/ivl/ivl %s %s %s %sVPI_MODULE_PATH=%s -o %s %s",
	  P_IF_SET(topmodule),
	  target.targetinfo.compsw, target.targetinfo.name,
	  VPImodpath.compsw, VPImodpath.name,
	  /* compileinfo.pform ? "-P whatever_pform" : "",
	     compileinfo.elabnetlist ? "-E whatever_netlist" : "", */
	  tmpCCfile,
	  tmpPPfile);
  if (compileinfo.debug) {
    printf("Executing command : \n%s\n", argument);
    printf("Removing file %s\n", tmpPPfile);
  } else {
    if (system(argument)) {
      fprintf(stderr, "Compilation failed. Terminating compilation\n");
      unlink(tmpCCfile);
      free(argument);
      exit(1);
    }
    unlink(tmpPPfile);
  }
  
  free(argument);
  return;
}


void postprocess(void)
{
  char *argument;

  /* Hopefully not filling stack with large array */
  argument = malloc(256);

  switch (target.target) {
  case VVM:   /* Compile C++ source */

    sprintf(argument, "g++ -rdynamic %s%s %s%s %s -o %s -lvvm -ldl",
	    cppincdir.compsw, cppincdir.name, /* Include dir */
	    cpplibdir.compsw, cpplibdir.name, /* Library dir */
	    tmpCCfile,
	    outputfile.name);
    if (compileinfo.debug) {
      printf("Executing command :\n%s\n", argument);
    } else {
      if (system(argument)) {
	fprintf(stderr, "g++ compilation failed. Terminating compilation\n");
	unlink(tmpCCfile);
	free(argument);
	exit(1);
      }
    }
    unlink(tmpCCfile);
    break;
  case XNF:   /* Just move file as is */
    if (compileinfo.debug) {
      printf("Moving file %s to %s\n", tmpCCfile, outputfile.name);
    } else {
      rename(tmpCCfile, outputfile.name);
    }
    break;
  case OTHER: /* Just move file as is */
    sprintf(outputfile.name, "%s.%s", outputfile.name, target.targetinfo.name);
    if (compileinfo.debug) {
      printf("Moving file %s to %s\n", tmpCCfile, outputfile.name);
    } else {
      rename(tmpCCfile, outputfile.name);
    }
    break;
  default:
    fprintf(stderr, "Illegal target. This should never happen.\n");
    error_exit();
  }
  
  if (compileinfo.execute) {
    if (compileinfo.debug) {
      printf("Executing command :\n%s\n", outputfile.name);
    } else {
      system(outputfile.name);
    }
  }


  free(argument);
  return;
}



int main(int argc, char **argv)
{
  const char optstring[] = "D:I:Xxf:o:s:t:EPvd";
  int optelem = 0;


  while ((optelem = getopt(argc, argv, optstring)) != EOF) {
    
    switch ((char)optelem) {
    case 'D': /* defines */
      sprintf(ivlppdefines.name, "%s %s%s", ivlppdefines.name, 
	      ivlppdefines.compsw, optarg);
      ivlppdefines.set =1;
      break;
    case 'I': /* includepath */
      sprintf(ivlppincdir.name, "%s %s %s", ivlppincdir.name, 
	      ivlppincdir.compsw, optarg);
      ivlppincdir.set = 1;
      break;
    case 'X': /* xnf target */
      if (target.targetinfo.set)
	fprintf(stderr, "Target already set to %s\n", target.targetinfo.name);
      else {
	sprintf(target.targetinfo.name,"%s xnf", target.targetinfo.compsw);
	target.targetinfo.set = 1;
	target.target = XNF;
      }
      break;
    case 'x': /* execute */
      compileinfo.execute = 1;
      break; 
    case 'f': /* flags? */
      sorry((char)optelem);
      break;
    case 'o': /* output file */
      if (outputfile.set) 
	fprintf(stderr, "Output filename already set to %s\n", 
		outputfile.name);
      else {
	sprintf(outputfile.name, "%s",  optarg);
	outputfile.set = 1;
      }
      break;
    case 's': /* top module */
      if (topmodule.set)
	fprintf(stderr, "Topmodule name already set to %s\n", topmodule.name);
      else {
	sprintf(topmodule.name, "%s %s", topmodule.compsw, optarg);
	topmodule.set = 1 ;
      }
      break;
    case 't': /* code target */
      if (target.targetinfo.set)
	fprintf(stderr, "Target already set to %s\n", target.targetinfo.name);
      else {
	sprintf(target.targetinfo.name, "%s", optarg);
	target.targetinfo.set = 1;
	target.target = resolvtarget(target.targetinfo.name);
      }
      break;
    case 'E': /* elaborated netlist */
      compileinfo.elabnetlist = 1;
      break;
    case 'P': /* dump pform */
      compileinfo.pform = 1;
      break;
    case 'v':
      printf("gverilog version %s\n", VERSION);
      system("ivlpp -v");
      printf("*****\n");
      system("ivl -v");
      printf("*****\n");
      system("g++ -v");
      return(0);
    case 'd':
      compileinfo.debug = 1;
      break;
    case '?':
      fprintf(stderr, "Not defined commandswitch %s\n", argv[optind]);
      error_exit();
    default:
      fprintf(stderr, "Not handled commandswitch %s\n", argv[optind]);
      error_exit();
    }

  }

  if (optind == argc){
    fprintf(stderr, "Missing infile(s)\n");
    error_exit();
  }

  /* Resolve temporary file storage */
  sprintf(tmpPPfile, "/tmp/ivl%d.pp", (int)getpid());
  sprintf(tmpCCfile, "/tmp/ivl%d.cc", (int)getpid());

  /* Build list of verilog files */
  for(; optind != argc; optind++) {
    sprintf(verilogfiles, "%s %s", verilogfiles, argv[optind]); 
  }

  /* Determine output filename if not explicitly set */
  if (!outputfile.set) {
    create_outputfilename(verilogfiles, outputfile.name);
  }


  preprocess();

  compile();

  postprocess();
  
  return(0);
}
