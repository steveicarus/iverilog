#ifndef __AStatement_H
#define __AStatement_H
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

# include  <map>
# include  "StringHeap.h"
# include  "LineInfo.h"

class PExpr;

class AStatement  : public LineInfo {

    public:
      AStatement() { }
      virtual ~AStatement() =0;

      virtual void dump(ostream&out, unsigned ind) const;

    private: // not implemented
      AStatement(const AStatement&);
      AStatement& operator= (const AStatement&);
};

class AContrib : public AStatement {

    public:
      AContrib();
      ~AContrib();

    private:
};

class AProcess : public LineInfo {

    public:
      enum Type { PR_INITIAL, PR_ALWAYS };

      AProcess(Type t, AStatement*st)
      : type_(t), statement_(st) { }

      ~AProcess();

      map<perm_string,PExpr*> attributes;

	// Dump the analog process
      void dump(ostream&out, unsigned ind) const;

    private:
      Type type_;
      AStatement*statement_;

    private: // not implemented
      AProcess(const AProcess&);
      AProcess& operator= (const AProcess&);
};

#endif
