/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include  "discipline.h"

nature_t::nature_t(perm_string name__, perm_string access__)
: name_(name__), access_(access__)
{
}

nature_t::~nature_t()
{
}

ivl_discipline_s::ivl_discipline_s(perm_string name__, ivl_dis_domain_t domain__,
				   nature_t*pot, nature_t*flow__)
: name_(name__), domain_(domain__), potential_(pot), flow_(flow__)
{
}

ivl_discipline_s::~ivl_discipline_s()
{
}
