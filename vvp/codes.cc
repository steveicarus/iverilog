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

# include  "codes.h"
# include  "statistics.h"
# include  <string.h>
# include  <assert.h>

/*
 * The code space is broken into chunks, to make for efficient
 * allocation of large amounts. Each chunk is an array of vvp_code_s
 * structures, with the last opcode loaded with an of_CHUNK_LINK
 * instruction to branch to the next chunk. This handles the case
 * where the program counter steps off the end of a chunk.
 */
const unsigned code_chunk_size = 1024;

static struct vvp_code_s *first_chunk = 0;
static struct vvp_code_s *current_chunk = 0;
static unsigned current_within_chunk = 0;

/*
 * This initializes the code space. It sets up the first code chunk,
 * and places at address 0 a ZOMBIE instruction.
 */
void codespace_init(void)
{
      assert(current_chunk == 0);
      first_chunk = new struct vvp_code_s [code_chunk_size];
      current_chunk = first_chunk;

      current_chunk[0].opcode = &of_ZOMBIE;

      current_chunk[code_chunk_size-1].opcode = &of_CHUNK_LINK;
      current_chunk[code_chunk_size-1].cptr = 0;

      current_within_chunk = 1;

      count_opcodes = 0;
      size_opcodes += code_chunk_size * sizeof (struct vvp_code_s);
}

vvp_code_t codespace_next(void)
{
      if (current_within_chunk == (code_chunk_size-1)) {
	    current_chunk[code_chunk_size-1].cptr
		  = new struct vvp_code_s [code_chunk_size];
	    current_chunk = current_chunk[code_chunk_size-1].cptr;

	      /* Put a link opcode on the end of the chunk. */
	    current_chunk[code_chunk_size-1].opcode = &of_CHUNK_LINK;
	    current_chunk[code_chunk_size-1].cptr   = 0;

	    current_within_chunk = 0;

	    size_opcodes += code_chunk_size * sizeof (struct vvp_code_s);
      }

      vvp_code_t res = current_chunk + current_within_chunk;
      return res;
}

vvp_code_t codespace_allocate(void)
{
      vvp_code_t res = codespace_next();
      current_within_chunk += 1;
      count_opcodes += 1;

      memset(res, 0, sizeof(*res));

      return res;
}

vvp_code_t codespace_null(void)
{
      return first_chunk + 0;
}

