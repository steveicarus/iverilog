#ifndef __verireal_H
#define __verireal_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: verireal.h,v 1.1 1999/06/15 02:50:02 steve Exp $"
#endif

class verireal {

    public:
      explicit verireal();
      explicit verireal(const char*text);
      ~verireal();

    private:
      bool sign_;
      unsigned long mant_;
      signed int exp10_;
};

/*
 * $Log: verireal.h,v $
 * Revision 1.1  1999/06/15 02:50:02  steve
 *  Add lexical support for real numbers.
 *
 */
#endif
