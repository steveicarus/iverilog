/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_pevent.cc,v 1.4 1999/12/12 19:47:54 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_gates.h"
# include  "vvm_thread.h"

vvm_sync::vvm_sync()
: hold_(0)
{
}

void vvm_sync::wait(vvm_thread*thr)
{
      assert(hold_ == 0);
      hold_ = thr;
}

void vvm_sync::wakeup()
{
      vvm_thread*tmp = hold_;
      hold_ = 0;
      if (tmp) tmp->thread_yield();
}


/*
 * $Log: vvm_pevent.cc,v $
 * Revision 1.4  1999/12/12 19:47:54  steve
 *  Remove the useless vvm_simulation class.
 *
 * Revision 1.3  1999/05/01 20:43:55  steve
 *  Handle wide events, such as @(a) where a has
 *  many bits in it.
 *
 *  Add to vvm the binary ^ and unary & operators.
 *
 *  Dump events a bit more completely.
 *
 * Revision 1.2  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */

