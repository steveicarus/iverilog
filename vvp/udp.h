#ifndef __udp_H
#define __udp_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

#include "functor.h"

// UDP implementation:

typedef struct udp_table_entry_s *udp_table_entry_t;

struct vvp_udp_s
{
      char *name;
      udp_table_entry_t table;
      unsigned ntable;
      unsigned sequ;
      unsigned nin;
      unsigned char init;

      void compile_table(char **tab);
      unsigned char propagate(functor_t fu, vvp_ipoint_t i);

    private:
      void compile_row_(udp_table_entry_t, char *);
};

struct vvp_udp_s *udp_create(char *label);
struct vvp_udp_s *udp_find(char *label);


// UDP instances:
/*
 * A complete UDP instance includes one udp_functor_s functor that
 * holds the output of the UDP, as well as the first 4 inputs. This
 * also points to the vvp_udp_s table that is the behavior for the
 * device.
 *
 * If there are more than 4 inputs to the device, then enough
 * edge_inputs_functor_s functors is created to receive all the
 * inputs. All the edge_inputs_functors_s ::out members point to the
 * leading udp_functor_s object, so the ::set methods all invoke the
 * ::set method of this functor.
 *
 *        +---+     First object is a udp_functor_s object
 *   <----+   +--
 *        |   +--
 *        |   +--
 *        |   +--
 *        +---+
 *          ^
 *          | +---+    Subsequent objects are edge_inputs_functor_s
 *          \-+   +--     that point their outputs to the leading
 *          ^ |   +--     udp_functor_s object. There are as many
 *          | |   +--     edge_inputs_functor_s objects as needed
 *          | |   +--     to accommodate all the inputs of the user
 *          | +---+       defined primitive.
 *          |
 *          | +---+
 *          \-+   +--
 *            |   +--
 *            |   +--
 *            |   +--
 *            +---+
 *              .
 *              .
 *              .
 * (The propagate method of the vvp_udp_s object relies on the fact
 * that the initial udp_functor_s and the subsequend
 * edge_inputs_functor_s objects are consecutive in functor space.)
 */
class udp_functor_s : public edge_inputs_functor_s
{
    public:
      explicit udp_functor_s(vvp_udp_s *u) : udp(u) {}
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);
      vvp_udp_s *udp;
};

#endif
