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
#ident "$Id: codes.cc,v 1.10 2002/07/05 02:50:58 steve Exp $"
#endif

# include  "codes.h"
# include  "statistics.h"
# include  <string.h>
# include  <assert.h>


const unsigned code_index0_size = 2 << 9;
const unsigned code_index1_size = 2 << 11;
const unsigned code_index2_size = 2 << 10;

struct code_index0 {
      struct vvp_code_s table[code_index0_size];
};

struct code_index1 {
      struct code_index0* table[code_index1_size];
};

static vvp_cpoint_t code_count = 0;
static struct code_index1*code_table[code_index2_size] = { 0 };

/*
 * This initializes the code space. It sets up a code table and places
 * at address 0 a ZOMBIE instruction.
 */
void codespace_init(void)
{
      code_table[0] = new struct code_index1;
      memset(code_table[0], 0, sizeof (struct code_index1));
      code_table[0]->table[0] = new struct code_index0;
      memset(code_table[0]->table[0], 0, sizeof(struct code_index0));

      vvp_code_t cp = code_table[0]->table[0]->table + 0;
      cp->opcode = &of_ZOMBIE;

      code_count = 1;
}

vvp_cpoint_t codespace_allocate(void)
{
      vvp_cpoint_t idx = code_count;

      idx /= code_index0_size;

      unsigned index1 = idx % code_index1_size;
      idx /= code_index1_size;

      assert(idx < code_index2_size);

      if (code_table[idx] == 0) {
	    code_table[idx] = new struct code_index1;
	    memset(code_table[idx], 0, sizeof(struct code_index1));
      }

      if (code_table[idx]->table[index1] == 0) {
	    code_table[idx]->table[index1] = new struct code_index0;
	    memset(code_table[idx]->table[index1],
		   0, sizeof(struct code_index0));
      }

      vvp_cpoint_t res = code_count;
      code_count += 1;
      count_opcodes += 1;
      return res;
}

vvp_cpoint_t codespace_next(void)
{
      return code_count;
}

unsigned code_limit(void)
{
      return code_count;
}

vvp_code_t codespace_index(vvp_cpoint_t point)
{
      assert(point < code_count);

      unsigned index0 = point % code_index0_size;
      point /= code_index0_size;

      unsigned index1 = point % code_index1_size;
      point /= code_index1_size;

      return code_table[point]->table[index1]->table + index0;
}


/*
 * $Log: codes.cc,v $
 * Revision 1.10  2002/07/05 02:50:58  steve
 *  Remove the vpi object symbol table after compile.
 *
 * Revision 1.9  2002/03/01 05:43:59  steve
 *  Initialize all the codes tables.
 *
 * Revision 1.8  2001/05/09 04:23:18  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.7  2001/04/13 03:55:18  steve
 *  More complete reap of all threads.
 *
 * Revision 1.6  2001/04/01 06:40:44  steve
 *  Support empty statements for hanging labels.
 *
 * Revision 1.5  2001/03/22 05:28:41  steve
 *  Add code label forward references.
 *
 * Revision 1.4  2001/03/22 05:08:00  steve
 *  implement %load, %inv, %jum/0 and %cmp/u
 *
 * Revision 1.3  2001/03/20 06:16:23  steve
 *  Add support for variable vectors.
 *
 * Revision 1.2  2001/03/11 23:06:49  steve
 *  Compact the vvp_code_s structure.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */

