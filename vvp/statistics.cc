/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: statistics.cc,v 1.5 2002/08/12 01:35:08 steve Exp $"
#endif

# include  "statistics.h"

/*
 * This is a count of the instruction opcodes that were created.
 */
unsigned long count_opcodes = 0;

unsigned long count_functors = 0;
unsigned long count_functors_table = 0;
unsigned long count_functors_bufif = 0;
unsigned long count_functors_resolv= 0;
unsigned long count_functors_var   = 0;

unsigned long count_vpi_nets = 0;

unsigned long count_vpi_memories = 0;

unsigned long count_vpi_scopes = 0;

size_t size_opcodes = 0;


/*
 * $Log: statistics.cc,v $
 * Revision 1.5  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/07/05 20:08:44  steve
 *  Count different types of functors.
 *
 * Revision 1.3  2002/07/05 17:14:15  steve
 *  Names of vpi objects allocated as vpip_strings.
 *
 * Revision 1.2  2002/07/05 03:46:43  steve
 *  Track opcode memory space.
 *
 * Revision 1.1  2002/07/05 02:50:58  steve
 *  Remove the vpi object symbol table after compile.
 *
 */

