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
#ident "$Id: vvm_thread.cc,v 1.1 1998/11/09 23:44:11 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_thread.h"

vvm_thread::vvm_thread(vvm_simulation*sim)
: sim_(sim)
{
      sim_->thread_active(this);
}

vvm_thread::~vvm_thread()
{
}

/*
 * $Log: vvm_thread.cc,v $
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */

