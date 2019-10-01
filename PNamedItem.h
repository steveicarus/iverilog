#ifndef IVL_PNamedItem_H
#define IVL_PNamedItem_H
/*
 * Copyright (c) 2019 Martin Whitaker (icarus@martin-whitaker.me.uk)
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

# include  "LineInfo.h"

/*
 * The PNamedItem class is the base class for all items that can be added
 * to a scope's local symbol map.
 */
class PNamedItem : virtual public LineInfo {

    public:
      enum SymbolType { ANY, PARAM, NET, VAR, GENVAR, EVENT, TYPE, ENUM,
                        CLASS, FUNCTION, TASK, BLOCK, GENBLOCK, MODPORT,
                        PACKAGE, MODULE, PROGRAM, INTERFACE, PRIMITIVE,
                        INSTANCE };

      explicit PNamedItem();
      virtual ~PNamedItem();

      virtual SymbolType symbol_type() const;
};

extern std::ostream& operator << (std::ostream&, PNamedItem::SymbolType);

/*
 * The PGenvar class represents a genvar. This is only used to represent
 * genvar in a scope's local symbol map.
 */
class PGenvar : public PNamedItem {

    public:
      explicit PGenvar();
      virtual ~PGenvar();

      SymbolType symbol_type() const;
};

#endif /* IVL_PNamedItem_H */
