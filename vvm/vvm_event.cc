/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm_event.cc,v 1.6 2001/07/25 03:10:50 steve Exp $"
#endif

# include "config.h"

# include  "vvm.h"
# include  <assert.h>

vvm_event::vvm_event()
{
      event_ = 0;
}

vvm_event::~vvm_event()
{
      assert(event_ == 0);
}

void vvm_event::schedule(unsigned long delay)
{
      event_ = vpip_sim_insert_event(delay, this, callback_, 0);
}

void vvm_event::callback_(void*cbd)
{
      vvm_event*obj = reinterpret_cast<vvm_event*>(cbd);
      obj->event_ = 0;
      obj->event_function();
      delete obj;
}

/*
 * $Log: vvm_event.cc,v $
 * Revision 1.6  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.5  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.4  2000/01/06 05:56:22  steve
 *  Cleanup and some asserts.
 *
 * Revision 1.3  1999/12/12 19:47:54  steve
 *  Remove the useless vvm_simulation class.
 *
 * Revision 1.2  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */

