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
//  SDW - Validate continuous <= in assignment.
//


module main;

reg globvar;

reg [3:0] var1;
reg error;

wire var2 = (4'h02 >= var1);

initial
  begin
    error = 0;
    var1 = 4'h0 ;
    #1 ;
    if(var2 !== 1'b1)
      begin
        $display("FAILED continuous >= logical op (1)");
        error = 1;
      end
    #1 ;
    var1 = 4'h2;
    #1 ;
    if(var2 !== 1'b1)
      begin
        $display("FAILED continuos <= logical op (2)");
        error = 1;
      end
    #1 ;
    var1 = 4'h4;
    #1 ;
    if(var2 !== 1'b0)
      begin
        $display("FAILED continuos <= logical op (3)");
        error = 1;
      end
    if(error == 0)
        $display("PASSED");
  end

endmodule // main
