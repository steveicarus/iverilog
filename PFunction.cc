/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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

# include "config.h"
# include "PTask.h"
# include "Statement.h"
# include "ivl_assert.h"

using namespace std;

PFunction::PFunction(perm_string name, LexicalScope*parent, bool is_auto__)
: PTaskFunc(name, parent), statement_(0)
{
      is_auto_ = is_auto__;
      return_type_ = 0;
}

PFunction::~PFunction()
{
}

void PFunction::set_statement(Statement*s)
{
      ivl_assert(*this, s != 0);
      ivl_assert(*this, statement_ == 0);
      statement_ = s;
}

void PFunction::push_statement_front(Statement*stmt)
{
        // This should not be possible.
      ivl_assert(*this, statement_);

	// Get the PBlock of the statement. If it is not a PBlock,
	// then create one to wrap the existing statement and the new
	// statement that we're pushing.
      PBlock*blk = dynamic_cast<PBlock*> (statement_);
      if (blk == 0) {
	    PBlock*tmp = new PBlock(PBlock::BL_SEQ);
	    tmp->set_line(*this);
	    vector<Statement*>tmp_list(1);
	    tmp_list[0] = statement_;
	    tmp->set_statement(tmp_list);

	    statement_ = tmp;
	    blk = tmp;
      }

	// Now do the push.
      blk->push_statement_front(stmt);
}

void PFunction::set_return(data_type_t*t)
{
      return_type_ = t;
}

PChainConstructor* PFunction::extract_chain_constructor()
{
      PChainConstructor*res = 0;

      if ((res = dynamic_cast<PChainConstructor*> (statement_))) {
	    statement_ = new PBlock(PBlock::BL_SEQ);
	    statement_->set_line(*this);

      } else if (PBlock*blk = dynamic_cast<PBlock*>(statement_)) {
	    res = blk->extract_chain_constructor();
      }

      return res;
}

PNamedItem::SymbolType PFunction::symbol_type() const
{
      return FUNCTION;
}


PLet::PLet(perm_string name, LexicalScope*parent, list<let_port_t*>*ports,
           PExpr*expr)
: PTaskFunc(name, parent), ports_(ports), expr_(expr)
{
}

PLet::~PLet()
{
}
