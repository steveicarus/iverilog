/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: debug.cc,v 1.1 2001/05/05 23:55:46 steve Exp $"
#endif

/*
 * This file provides a simple command line debugger for the vvp
 * runtime. It is a means to interract with the user running the
 * simulation.
 */

# include  "config.h"

#if defined(WITH_DEBUG)

# include  "debug.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <readline/readline.h>
# include  <readline/history.h>
# include  <string.h>
# include  <malloc.h>


static bool interact_flag = false;

static void cmd_go(unsigned, char*[])
{
      interact_flag = false;
}

static void cmd_time(unsigned, char*[])
{
      unsigned long ticks = schedule_simtime();
      printf("%lu ticks\n", ticks);
}

static void cmd_unknown(unsigned argc, char*argv[])
{
      printf("Unknown command: %s\n", argv[0]);
}

struct {
      const char*name;  void (*proc)(unsigned argc, char*argv[]);
} cmd_table[] = {
      { "go",   &cmd_go},
      { "time", &cmd_time},
      { 0,      &cmd_unknown}
};

void breakpoint(void)
{
      printf("** VVP Interactive Debugger **\n");
      printf("Current simulation time is %lu ticks.\n", schedule_simtime());

      interact_flag = true;
      while (interact_flag) {
	    char*input = readline("> ");
	    unsigned argc = 0;
	    char**argv = new char*[strlen(input)/2];

	    for (char*cp = input+strspn(input, " ")
		       ; *cp; cp += strspn(cp, " ")) { 
		  argv[argc] = cp;

		  cp += strcspn(cp, " ");
		  if (*cp)
			*cp++ = 0;

		  argc += 1;
	    }

	    if (argc > 0) {
		  unsigned idx;
		  for (idx = 0 ;  cmd_table[idx].name ;  idx += 1)
			if (strcmp(cmd_table[idx].name, argv[0]) == 0)
			      break;

		  cmd_table[idx].proc (argc, argv);
	    }

	    delete[] argv;
	    free(input);
      }
}

#endif
/*
 * $Log: debug.cc,v $
 * Revision 1.1  2001/05/05 23:55:46  steve
 *  Add the beginnings of an interactive debugger.
 *
 */

