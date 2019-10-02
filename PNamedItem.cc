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

# include  "PNamedItem.h"
# include  <ostream>

PNamedItem::PNamedItem()
{
}

PNamedItem::~PNamedItem()
{
}

PNamedItem::SymbolType PNamedItem::symbol_type() const
{
      return ANY;
}

std::ostream& operator << (std::ostream&o, PNamedItem::SymbolType st)
{
      switch (st) {
          case PNamedItem::ANY:
            o << "a symbol";
            break;
          case PNamedItem::PARAM:
            o << "a parameter";
            break;
          case PNamedItem::NET:
            o << "a net";
            break;
          case PNamedItem::VAR:
            o << "a variable";
            break;
          case PNamedItem::GENVAR:
            o << "a genvar";
            break;
          case PNamedItem::EVENT:
            o << "an event";
            break;
          case PNamedItem::TYPE:
            o << "a type";
            break;
          case PNamedItem::ENUM:
            o << "an enum type or value";
            break;
          case PNamedItem::CLASS:
            o << "a class";
            break;
          case PNamedItem::FUNCTION:
            o << "a function";
            break;
          case PNamedItem::TASK:
            o << "a task";
            break;
          case PNamedItem::BLOCK:
            o << "a named block";
            break;
          case PNamedItem::GENBLOCK:
            o << "a generate block";
            break;
          case PNamedItem::MODPORT:
            o << "a modport";
            break;
          case PNamedItem::PACKAGE:
            o << "a package";
            break;
          case PNamedItem::MODULE:
            o << "a module";
            break;
          case PNamedItem::PROGRAM:
            o << "a program";
            break;
          case PNamedItem::INTERFACE:
            o << "an interface";
            break;
          case PNamedItem::PRIMITIVE:
            o << "a primitive";
            break;
          case PNamedItem::INSTANCE:
            o << "an instance name";
            break;
          default:
            break;
      }
      return o;
}

PGenvar::PGenvar()
{
}

PGenvar::~PGenvar()
{
}

PNamedItem::SymbolType PGenvar::symbol_type() const
{
      return GENVAR;
}
