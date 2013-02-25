/*
 * Copyright (c) 1999-2008,2010 Stephen Williams (steve@icarus.com)
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
# include  "PTask.h"
# include  <cassert>

PTask::PTask(perm_string name, LexicalScope*parent, bool is_auto__)
: PScope(name, parent), this_type_(0), ports_(0), statement_(0)
{
      is_auto_ = is_auto__;
}

PTask::~PTask()
{
}

void PTask::set_this(class_type_t*type)
{
      assert(this_type_ == 0);
      this_type_ = type;
}

void PTask::set_ports(vector<PWire*>*p)
{
      assert(ports_ == 0);
      ports_ = p;
}

void PTask::set_statement(Statement*s)
{
      assert(statement_ == 0);
      statement_ = s;
}
