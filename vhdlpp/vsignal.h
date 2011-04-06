#ifndef __vsignal_H
#define __vsignal_H
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "StringHeap.h"
# include  "LineInfo.h"

class Architecture;
class Entity;
class VType;

class Signal : public LineInfo {

    public:
      Signal(perm_string name, const VType*type);
      ~Signal();

      int emit(ostream&out, Entity*ent, Architecture*arc);

      void dump(ostream&out, int indent = 0) const;

    private:
      perm_string name_;
      const VType*type_;

    private: // Not implemented
      Signal(const Signal&);
      Signal& operator = (const Signal&);
};

#endif
