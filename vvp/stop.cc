/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: stop.cc,v 1.2 2003/02/22 06:32:36 steve Exp $"
#endif

/*
 * This file provides a simple command line debugger for the vvp
 * runtime. It is a means to interact with the user running the
 * simulation.
 */

# include  "config.h"


# include  "vpi_priv.h"
# include  "vthread.h"
# include  "schedule.h"
# include  <stdio.h>
#ifdef HAVE_LIBREADLINE
# include  <readline/readline.h>
# include  <readline/history.h>
#endif
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

static bool interact_flag = true;

static void invoke_systf(const char*txt)
{
      printf("**** System invocation not supported yet.\n");
}

static void cmd_call(unsigned argc, char*argv[])
{
      if (argc <= 1)
	    return;

      vpiHandle call_handle = vpip_build_vpi_call(argv[1], 0, 0, 0, 0);
      if (call_handle == 0)
	    return;

      vpip_execute_vpi_call(0, call_handle);

      vpi_free_object(call_handle);
}

static void cmd_cont(unsigned, char*[])
{
      interact_flag = false;
}

static void cmd_finish(unsigned, char*[])
{
      interact_flag = false;
      schedule_finish(0);
}

static void cmd_help(unsigned, char*[]);

static void cmd_time(unsigned, char*[])
{
      unsigned long ticks = schedule_simtime();
      printf("%lu ticks\n", ticks);
}

static void cmd_unknown(unsigned, char*argv[])
{
      printf("Unknown command: %s\n", argv[0]);
      printf("Try the help command to get a summary\n"
	     "of available commands.\n");
}

struct {
      const char*name; 
      void (*proc)(unsigned argc, char*argv[]);
      const char*summary;
} cmd_table[] = {
      { "call",   &cmd_call,
        "Call a system task"},
      { "cont",   &cmd_cont,
        "Resume (continue) the simulation"},
      { "finish", &cmd_finish,
        "Finish the simulation."},
      { "help",  &cmd_help,
        "Get help."},
      { "time",   &cmd_time,
        "Print the current simulation time."},
      { 0,        &cmd_unknown, 0}
};

static void cmd_help(unsigned argc, char*argv[])
{
      printf("Commands can be from the following table of base commands,\n"
	     "or can be invocations of system tasks/functions.\n\n");
      for (unsigned idx = 0 ;  cmd_table[idx].name != 0 ;  idx += 1) {
	    printf("%-8s - %s\n", cmd_table[idx].name, cmd_table[idx].summary);
      }

      printf("\nIf the command starts with a dollar ($) character, it\n"
	     "is taken instead to be a system task or function call\n"
	     "and will be parsed and invoked accordingly.\n");
}


static void invoke_command(char*txt)
{
      unsigned argc = 0;
      char**argv = new char*[strlen(txt)/2];

	/* Chop the line into words. */
      for (char*cp = txt+strspn(txt, " ")
		 ; *cp; cp += strspn(cp, " ")) { 
	    argv[argc] = cp;

	    cp += strcspn(cp, " ");
	    if (*cp)
		  *cp++ = 0;

	    argc += 1;
      }

	/* Look up the command, using argv[0] as the key. */
      if (argc > 0) {
	    unsigned idx;
	    for (idx = 0 ;  cmd_table[idx].name ;  idx += 1)
		  if (strcmp(cmd_table[idx].name, argv[0]) == 0)
			break;

	    cmd_table[idx].proc (argc, argv);
      }


      delete[]argv;
}

void stop_handler(int rc)
{
      printf("** VVP Stop(%d) **\n", rc);
      printf("** Current simulation time is %lu ticks.\n", schedule_simtime());

      interact_flag = true;
      while (interact_flag) {
	    char*input = readline("> ");
	    if (input == 0)
		  break;

	      /* Advance to the first input character. */
	    char*first = input;
	    while (*first && isspace(*first))
		  first += 1;

	      /* If the line starts with a $, then this is the
		 invocation of a system task or system function, so
		 call the invoke_systf function to handle
		 it. Otherwise, this is a simple command, so invoke
		 the command processor. */

	    if (*first == '$') {
		  invoke_systf(first);

	    } else {
		  invoke_command(first);
	    }

	    free(input);
      }

      printf("** Continue **\n");
}

/*
 * $Log: stop.cc,v $
 * Revision 1.2  2003/02/22 06:32:36  steve
 *  Basic support for system task calls.
 *
 * Revision 1.1  2003/02/21 03:40:35  steve
 *  Add vpiStop and interactive mode.
 *
 */

