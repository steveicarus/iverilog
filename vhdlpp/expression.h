#ifndef __expression_H
#define __expression_H
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

/*
 * The Expression class represents parsed expressions from the parsed
 * VHDL input. The Expression class is a virtual class that holds more
 * specific derived expression types.
 */
class Expression : public LineInfo {

    public:
      Expression();
      virtual ~Expression() =0;

      virtual void dump(ostream&out, int indent) const;

    private:

    private: // Not implemented
      Expression(const Expression&);
      Expression& operator = (const Expression&);
};

class ExpLogical : public Expression {

    public:
      enum fun_t { AND, OR, NAND, NOR, XOR, XNOR };

    public:
      ExpLogical(ExpLogical::fun_t ty, Expression*op1, Expression*op2);
      ~ExpLogical();

      void dump(ostream&out, int indent) const;

    private:
      fun_t fun_;
      Expression*operand1_;
      Expression*operand2_;
};

/*
 * The ExpName class represents an expression that is an identifier or
 * other sort of name.
 */
class ExpName : public Expression {

    public:
      ExpName(perm_string nn);
      ~ExpName();

      void dump(ostream&out, int indent) const;

    private:
      perm_string name_;
};

#endif
