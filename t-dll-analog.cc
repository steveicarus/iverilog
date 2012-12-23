/*
 * Copyright (c) 2008-2010 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.will need a Picture Elements Binary Software
 *    License.
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

# include "config.h"

# include  <iostream>

# include  <cstring>
# include  "target.h"
# include  "ivl_target.h"
# include  "compiler.h"
# include  "t-dll.h"
# include  <cstdlib>
# include  "ivl_alloc.h"

bool dll_target::process(const NetAnalogTop*net)
{
      bool rc_flag = true;

      ivl_process_t obj = (struct ivl_process_s*)
	    calloc(1, sizeof(struct ivl_process_s));

      obj->type_ = net->type();
      obj->analog_flag = 1;

      FILE_NAME(obj, net);

	/* Save the scope of the process. */
      obj->scope_ = lookup_scope_(net->scope());

      obj->nattr = net->attr_cnt();
      obj->attr = fill_in_attributes(net);

      assert(stmt_cur_ == 0);
      stmt_cur_ = (struct ivl_statement_s*)calloc(1, sizeof*stmt_cur_);
      rc_flag = net->statement()->emit_proc(this) && rc_flag;

      assert(stmt_cur_);
      obj->stmt_ = stmt_cur_;
      stmt_cur_ = 0;

	/* Save the process in the design. */
      obj->next_ = des_.threads_;
      des_.threads_ = obj;

      return rc_flag;
}

bool dll_target::proc_contribution(const NetContribution*net)
{
      assert(stmt_cur_);
      assert(stmt_cur_->type_ == IVL_ST_NONE);
      FILE_NAME(stmt_cur_, net);

      stmt_cur_->type_ = IVL_ST_CONTRIB;

      assert(expr_ == 0);
      net->lval()->expr_scan(this);
      stmt_cur_->u_.contrib_.lval = expr_;
      expr_ = 0;

      net->rval()->expr_scan(this);
      stmt_cur_->u_.contrib_.rval = expr_;
      expr_ = 0;

      return true;
}
