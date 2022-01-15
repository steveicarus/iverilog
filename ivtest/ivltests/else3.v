//
// Copyright (c) 1999 Steven Wilson (stevew@home.com)
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
//  SDW - Compound ifdef test with else, exterior define
//


module ifdef1;

reg error ;


`ifdef DOUBLE
initial
   begin
     #20;
     error = 1;
     #20;
   end
`else
`ifdef NOCODE
initial
   begin
     #20;
     error = 1;
     #20;
   end
`else
initial
   begin
     #20;
     error = 0;
     #20;
   end
`endif
`endif

initial
 begin
   #1;
   error = 1;
   #40;
   if(error == 0)
      $display("PASSED");
   else
      $display("FAILED");
  end

endmodule // main
