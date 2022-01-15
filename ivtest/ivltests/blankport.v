//
// Copyright (c) 1999 Stephan Boettcher (stephan@nevis1.nevis.columbia.edu)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//
//  SDW - PR 204 report - validates correct use of blank ports.

module none;
   reg x;
endmodule // none

module empty();
   reg x;
endmodule // none

module one (a);
   input a;
   reg x;
endmodule // one

module two (a, b);
   input a, b;
   reg x;
endmodule // two

module three (a, b, c);
   input a, b, c;
   reg x;
endmodule // two

module main;

   wire w1, w2, w3, w4, w5, w6, w7, w8, w9;

   none    U1 ();
   empty   U2 ();
   one     U3 ();
   one     U4 (w1);
   one     U5 (.a(w2));
   two     U6 ();
   two     U7 (,);
   two     U8 (w3,);
   two     U9 (,w4);
   two     Ua (w5,w6);
   two     Ub (.a(w7));
   two     Uc (.b(w8));
   two     Ud (.b(w8),.a(w9));
   three   Ue ();
   //three   Uf (,);  //XXXX I doubt this is legal... ?
   three   Ug (,,);

   initial $display("PASSED");

endmodule // main
