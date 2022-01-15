/*
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

// $Id: ldelay5.v,v 1.1 2001/12/26 23:45:57 sib4 Exp $

// Test for delays in structural logic.  Multiple UDP instances.

module test;

   wire [1:2] q, a, b;
   drec U1(q[1], a[1], b[1]);
   drec U2(q[2], a[2], b[2]);

   initial $display("PASSED");

endmodule

module drec (q, a, b);
   output q;
   input  a, b;
   U_drec #1 U(q, a, b);
endmodule

primitive U_drec (q, a, b);
   output        q;
   input  a, b;
   table
          1  0 : 1 ;
          0  1 : 0 ;
   endtable
endprimitive
