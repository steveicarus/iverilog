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
#ident "$Id: debug.cc,v 1.9 2002/07/05 04:40:59 steve Exp $"
#endif

/*
 * This file provides a simple command line debugger for the vvp
 * runtime. It is a means to interract with the user running the
 * simulation.
 */

# include  "config.h"

#if defined(WITH_DEBUG)

# include  "debug.h"
# include  "functor.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <readline/readline.h>
# include  <readline/history.h>
# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif


static bool interact_flag = false;

static void cmd_lookup(unsigned argc, char*argv[])
{
      printf(" **** not implemented...\n");
}

static void cmd_fbreak(unsigned argc, char*argv[])
{
      for (unsigned idx = 1 ;  idx < argc ;  idx += 1) {
	    vvp_ipoint_t fnc = strtoul(argv[idx],0,0);
	    functor_t fp = functor_index(fnc);

	    if (fp == 0) {
		  continue;
	    }

	    fp->break_flag = 1;
      }
}

static const char bitval_tab[4] = { '0', '1', 'x', 'z' };
static const char*strength_tab[8] = {
      "hiz",    "small",
      "medium", "weak",
      "large",  "pull",
      "strong", "supply" };

static void cmd_functor(unsigned argc, char*argv[])
{
      for (unsigned idx = 1 ;  idx < argc ;  idx += 1) {
	    vvp_ipoint_t fnc = strtoul(argv[idx],0,0);
	    functor_t fp = functor_index(fnc);

	    if (fp == 0) {
		  continue;
	    }

	    fp->debug_print(fnc);
      }
}

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
      { "fbreak", &cmd_fbreak},
      { "func",   &cmd_functor},
      { "go",     &cmd_go},
      { "lookup", &cmd_lookup},
      { "time",   &cmd_time},
      { 0,        &cmd_unknown}
};

void breakpoint(void)
{
      printf("** VVP Interactive Debugger **\n");
      printf("Current simulation time is %lu ticks.\n", schedule_simtime());

      interact_flag = true;
      while (interact_flag) {
	    char*input = readline("> ");
	    if (input == 0)
		  break;

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
 * Revision 1.9  2002/07/05 04:40:59  steve
 *  Symbol table uses more efficient key string allocator,
 *  and remove all the symbol tables after compile is done.
 *
 * Revision 1.8  2001/12/18 05:32:11  steve
 *  Improved functor debug dumps.
 *
 * Revision 1.7  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.6  2001/08/09 22:25:30  steve
 *  Include functor address in debug functor print.
 *
 * Revision 1.5  2001/07/19 02:20:55  steve
 *  EOF is the same as resume.
 *
 * Revision 1.4  2001/05/31 04:12:43  steve
 *  Make the bufif0 and bufif1 gates strength aware,
 *  and accurately propagate strengths of outputs.
 *
 * Revision 1.3  2001/05/30 03:02:35  steve
 *  Propagate strength-values instead of drive strengths.
 *
 * Revision 1.2  2001/05/08 23:32:26  steve
 *  Add to the debugger the ability to view and
 *  break on functors.
 *
 *  Add strengths to functors at compile time,
 *  and Make functors pass their strengths as they
 *  propagate their output.
 *
 * Revision 1.1  2001/05/05 23:55:46  steve
 *  Add the beginnings of an interactive debugger.
 *
 */

