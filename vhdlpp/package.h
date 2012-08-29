#ifndef __package_H
#define __package_H
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "scope.h"
# include  "LineInfo.h"
# include  <iostream>

class Package : public Scope, public LineInfo {

    public:
      Package(perm_string name, const ScopeBase&ref);
      ~Package();

      perm_string name() const { return name_; }

	// This method writes a package header to a library file.
      void write_to_stream(std::ostream&fd) const;

    private:
      perm_string name_;
};

#endif
