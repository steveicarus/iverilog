/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  "compiler.h"
# include  <typeinfo>
# include  "ivl_assert.h"

using namespace std;

NetExpr* NetFuncDef::evaluate_function(const LineInfo&loc, const std::vector<NetExpr*>&args) const
{
	// Make the context map;
      map<perm_string,NetExpr*>::iterator ptr;
      map<perm_string,NetExpr*>context_map;

      if (debug_eval_tree) {
	    cerr << loc.get_fileline() << ": debug: "
		 << "Evaluate function " << scope_->basename() << endl;
      }

	// Put the return value into the map...
      context_map[scope_->basename()] = 0;
	// Load the input ports into the map...
      ivl_assert(loc, ports_.size() == args.size());
      for (size_t idx = 0 ; idx < ports_.size() ; idx += 1) {
	    NetExpr*tmp = args[idx]->dup_expr();
	    perm_string aname = ports_[idx]->name();
	    context_map[aname] = tmp;

	    if (debug_eval_tree) {
		  cerr << loc.get_fileline() << ": debug: "
		       << "   input " << aname << " = " << *tmp << endl;
	    }
      }

	// Perform the evaluation
      bool flag = statement_->evaluate_function(loc, context_map);

	// Extract the result...
      ptr = context_map.find(scope_->basename());
      NetExpr*res = ptr->second;
      context_map.erase(ptr);


	// Cleanup the rest of the context.
      for (ptr = context_map.begin() ; ptr != context_map.end() ; ++ptr) {
	    delete ptr->second;
      }

	// Done.
      if (flag)
	    return res;

      delete res;
      return 0;
}

NetExpr* NetExpr::evaluate_function(const LineInfo&,
				    map<perm_string,NetExpr*>&) const
{
      cerr << get_fileline() << ": sorry: I don't know how to evaluate this expression at compile time." << endl;
      cerr << get_fileline() << ":      : Expression type:" << typeid(*this).name() << endl;

      return 0;
}

bool NetProc::evaluate_function(const LineInfo&,
				map<perm_string,NetExpr*>&) const
{
      cerr << get_fileline() << ": sorry: I don't know how to evaluate this statement at compile time." << endl;
      cerr << get_fileline() << ":      : Statement type:" << typeid(*this).name() << endl;

      return false;
}

bool NetAssign::evaluate_function(const LineInfo&loc,
				 map<perm_string,NetExpr*>&context_map) const
{
      if (l_val_count() != 1) {
	    cerr << get_fileline() << ": sorry: I don't know how to evaluate "
		  "concatenated l-values here." << endl;
	    return false;
      }

      const NetAssign_*lval = l_val(0);

      ivl_assert(loc, lval->word() == 0);
      ivl_assert(loc, lval->get_base() == 0);

      NetExpr*rval_result = rval()->evaluate_function(loc, context_map);
      if (rval_result == 0)
	    return false;

      map<perm_string,NetExpr*>::iterator ptr = context_map.find(lval->name());
      if (ptr->second)
	    delete ptr->second;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: "
		 << "NetAssign::evaluate_function: " << lval->name()
		 << " = " << *rval_result << endl;
      }

      ptr->second = rval_result;

      return true;
}

/*
 * Evaluating a NetBlock in a function is a simple matter of
 * evaluating the statements in order.
 */
bool NetBlock::evaluate_function(const LineInfo&loc,
				 map<perm_string,NetExpr*>&context_map) const
{
      bool flag = true;
      NetProc*cur = last_;
      do {
	    cur = cur->next_;
	    bool cur_flag = cur->evaluate_function(loc, context_map);
	    flag = flag && cur_flag;
      } while (cur != last_);

      return flag;
}

bool NetWhile::evaluate_function(const LineInfo&loc,
				map<perm_string,NetExpr*>&context_map) const
{
      bool flag = true;

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: NetWhile::evaluate_fuction: "
		 << "Start loop" << endl;
      }

      while (flag) {
	      // Evaluate the condition expression to try and get the
	      // condition for the loop.
	    NetExpr*cond = cond_->evaluate_function(loc, context_map);
	    if (cond == 0) {
		  flag = false;
		  break;
	    }

	    NetEConst*cond_const = dynamic_cast<NetEConst*> (cond);
	    ivl_assert(loc, cond_const);

	    long val = cond_const->value().as_long();
	    delete cond;

	      // If the condition is false, then break.
	    if (val == 0)
		  break;

	      // The condition is true, so evalutate the statement
	      // another time.
	    bool tmp_flag = proc_->evaluate_function(loc, context_map);
	    if (! tmp_flag)
		  flag = false;
      }

      if (debug_eval_tree) {
	    cerr << get_fileline() << ": debug: NetWhile::evaluate_fuction: "
		 << "Done loop" << endl;
      }

      return flag;
}

NetExpr* NetEBComp::evaluate_function(const LineInfo&loc,
				      map<perm_string,NetExpr*>&context_map) const
{
      NetExpr*lval = left_->evaluate_function(loc, context_map);
      NetExpr*rval = right_->evaluate_function(loc, context_map);

      if (lval == 0 || rval == 0) {
	    delete lval;
	    delete rval;
	    return 0;
      }

      NetEConst*res = eval_arguments_(lval, rval);
      delete lval;
      delete rval;
      return res;
}

NetExpr* NetEBAdd::evaluate_function(const LineInfo&loc,
				      map<perm_string,NetExpr*>&context_map) const
{
      NetExpr*lval = left_->evaluate_function(loc, context_map);
      NetExpr*rval = right_->evaluate_function(loc, context_map);

      if (lval == 0 || rval == 0) {
	    delete lval;
	    delete rval;
	    return 0;
      }

      NetExpr*res = eval_arguments_(lval, rval);
      delete lval;
      delete rval;
      return res;
}

NetExpr* NetEBShift::evaluate_function(const LineInfo&loc,
				      map<perm_string,NetExpr*>&context_map) const
{
      NetExpr*lval = left_->evaluate_function(loc, context_map);
      NetExpr*rval = right_->evaluate_function(loc, context_map);

      if (lval == 0 || rval == 0) {
	    delete lval;
	    delete rval;
	    return 0;
      }

      NetEConst*res = eval_arguments_(lval, rval);
      delete lval;
      delete rval;
      return res;
}

NetExpr* NetEConst::evaluate_function(const LineInfo&,
				      map<perm_string,NetExpr*>&) const
{
      NetEConst*res = new NetEConst(value_);
      res->set_line(*this);
      return res;
}

NetExpr* NetESignal::evaluate_function(const LineInfo&,
				       map<perm_string,NetExpr*>&context_map) const
{
      if (word_) {
	    cerr << get_fileline() << ": sorry: I don't know how to evaluate signal word selects at compile time." << endl;
	    return 0;
      }

      map<perm_string,NetExpr*>::iterator ptr = context_map.find(name());
      if (ptr == context_map.end()) {
	    cerr << get_fileline() << ": error: Cannot evaluate " << name()
		 << " in this context." << endl;
	    return 0;
      }

      return ptr->second->dup_expr();
}
