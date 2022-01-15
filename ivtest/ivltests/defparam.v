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
//  SDW - Validate defparam with list
//

module NameA ();

parameter ident0 = 12;
parameter ident1 = 20 ;

wire [31:0] value0 = ident0;
wire [31:0] value1 = ident1;

endmodule

module main ();

defparam main.testmodA.ident0 = 15;    // Validate single val
defparam main.testmodB.ident1 = 16,    // Validate list of vals
         main.testmodB.ident0 = 17;    // Validate single val

reg error;

NameA testmodA ();
NameA testmodB ();

initial
  begin
     error = 0;
     # 1;
     if(main.testmodA.value0 !== 15)
       begin
         error = 1;
         $display("FAILED - defparam.v main.testmodA.value0 != 15");
       end
     # 1;
     if(main.testmodA.value1 !== 20)
       begin
         error = 1;
         $display("FAILED - defparam.v main.testmodA.value1 != 20");
       end
     # 1;
     if(main.testmodB.value0 !== 17)
       begin
         error = 1;
         $display("FAILED - defparam.v main.testmodB.value0 != 17");
       end
     # 1;
     if(main.testmodB.value1 !== 16)
       begin
         error = 1;
         $display("FAILED - defparam.v main.testmodB.value1 != 16");
       end
     # 1;
     if(error == 0)
       $display("PASSED");
  end


endmodule
