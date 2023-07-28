/*
 * Copyright (c) 2023 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2023 Leo Moser (leo.moser@pm.me)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "tchk.h"

# include  "compile.h"
# include  "event.h"

#include <iostream>

__vpiTchk::__vpiTchk( __vpiScope *scope, unsigned file_idx, unsigned lineno)
: scope_(scope), file_idx_(file_idx), lineno_(lineno)
{
}

__vpiTchk::~__vpiTchk()
{
}

__vpiTchkWidth::__vpiTchkWidth( __vpiScope *scope, unsigned file_idx, unsigned lineno, vvp_fun_tchk_width* fun )
: __vpiTchk(scope, file_idx, lineno), fun_(fun)
{
}

__vpiTchkWidth::~__vpiTchkWidth()
{
}

int __vpiTchkWidth::vpi_get(int code) {

     switch (code) {

	  case vpiTchkType:
	    return vpiWidth;

	  default:
	    fprintf(stderr, "VPI error: unknown signal_get property %d.\n",
	            code);
	    return vpiUndefined;
      }

};

vpiHandle __vpiTchkWidth::vpi_handle(int code) {

      switch (code) {

	  case vpiScope:
	    return scope_;

	  case vpiTchkNotifier:
	    return vpi_notifier_;

	  case vpiTchkRefTerm:
	    if (!vpi_tchk_ref_term_) {
	        if (fun_->start_edge_ == vvp_edge_posedge) vpi_tchk_ref_term_ = new __vpiTchkRefTerm(vpiPosedge, vpi_reference_);
	        else vpi_tchk_ref_term_ = new __vpiTchkRefTerm(vpiNegedge, vpi_reference_);
	    }
	    return vpi_tchk_ref_term_;

	    // There is no data term for $width
	  case vpiTchkDataTerm:
	    return 0;
      }

      return 0;
};

void __vpiTchkWidth::vpi_get_delays(p_vpi_delay delays) {

        // $width has one limit TODO or also threshold?
        if (delays->no_of_delays != 1) return;

        delays->da[0].real = vpip_time_to_scaled_real(fun_->limit_, scope_);
};

void __vpiTchkWidth::vpi_put_delays(p_vpi_delay delays) {

        // $width has one limit TODO or also threshold?
        if (delays->no_of_delays != 1) return;

        fun_->limit_ = vpip_scaled_real_to_time64(delays->da[0].real, scope_);
};

__vpiTchkTerm::__vpiTchkTerm(int edge, vpiHandle expr)
:  expr_(expr), edge_(edge)
{
}

__vpiTchkTerm::~__vpiTchkTerm()
{
}

int __vpiTchkTerm::vpi_get(int code)
{
      switch (code) {
	  case vpiEdge:
	    return edge_;
	  default:
	    return 0;
      }
}

vpiHandle __vpiTchkTerm::vpi_handle(int code)
{
      switch (code) {
	  case vpiExpr:
	    return expr_;
	  default:
	    return nullptr;
      }
}

__vpiTchkRefTerm::__vpiTchkRefTerm(int edge, vpiHandle expr)
: __vpiTchkTerm(edge, expr)
{
}

__vpiTchkRefTerm::~__vpiTchkRefTerm()
{
}

__vpiTchkDataTerm::__vpiTchkDataTerm(int edge, vpiHandle expr)
: __vpiTchkTerm(edge, expr)
{
}

__vpiTchkDataTerm::~__vpiTchkDataTerm()
{
}

vpiHandle vpip_make_tchk_width(long file_idx, long lineno, vvp_fun_tchk_width* fun)
{
      __vpiTchkWidth* obj = new __vpiTchkWidth(vpip_peek_current_scope(), file_idx, lineno, fun);
      fun->vpi_tchk = obj;

      return obj;
}
