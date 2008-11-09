#ifndef __ivl_target_priv_H
#define __ivl_target_priv_H
/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include  <inttypes.h>

/*
* This deader has declarations related to the ivl_target.h API that
* are not to be exported outside of the core via the ivl_target.h
* interface.
*/

/*
* Information about islands. Connected branches within a net are
* collected into islands. Branches that are purely ddiscrete do not
* have disciplines and do not belong to islands.
*/

class ivl_discipline_s;

struct ivl_island_s {
      ivl_discipline_s*discipline;
	// user accessible flags. They are initially false, always.
      vector<bool> flags;
};

#endif
