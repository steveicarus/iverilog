#ifndef __vvp_vpi_callback_H
#define __vvp_vpi_callback_H
/*
 * Copyright (c) 2009 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vpi_user.h"

/*
 * Things derived from vvp_vpi_callback may have callbacks
 * attached. This is how vpi callbacks are attached to the vvp
 * structure.
 *
 * Things derived from vvp_vpi_callback may also be array'ed, so it
 * includes some members that arrays use.
 */
class vvp_vpi_callback {

    public:
      vvp_vpi_callback();
      virtual ~vvp_vpi_callback();

      void attach_as_word(class __vpiArray* arr, unsigned long addr);

      void add_vpi_callback(struct __vpiCallback*);
#ifdef CHECK_WITH_VALGRIND
	/* This has only been tested at EOS. */
      void clear_all_callbacks(void);
#endif

	// Derived classes implement this method to provide a way for
	// vpi to get at the vvp value of the object.
      virtual void get_value(struct t_vpi_value*value) =0;

    protected:
	// Derived classes call this method to indicate that it is
	// time to call the callback.
      void run_vpi_callbacks();

    private:
      struct __vpiCallback*vpi_callbacks_;
      class __vpiArray* array_;
      unsigned long array_word_;
};


#endif
