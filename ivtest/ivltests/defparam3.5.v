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
//  SDW - Validate defparam

module module_a (out0,in0);

input		in0;
output [5:0]	out0;

parameter [5:0] ident0 = 0;
parameter [5:0] ident1 = 5'h11;

reg [5:0] out0;

// Basic MUX switches on in0
always @ (in0)
   begin
     if(in0)
       out0 = ident0;
     else
       out0 = ident1;
   end

endmodule // module_a

module module_b (out0,out1,in0,in1);

input		in0;
input		in1;
output [5:0]	out0;
output [5:0]	out1;

module_a testmodA (.out0(out0),.in0(in0));
module_a testmodB (.out0(out1),.in0(in1));

endmodule // module_b

module main ();

reg        in0,in1;
reg	   error;
wire [5:0] out0,out1;

defparam NameB.testmodA.ident0 = 5'h4;
defparam NameB.testmodB.ident0 = 5'h5;
defparam NameB.testmodB.ident1 = 5'h6;

module_b NameB (.out0(out0),.out1(out1),
                .in0(in0),.in1(in1));


initial
  begin
     error = 0;
     #1 ;
     in0 = 0;
     #1 ;
     if(out0 != 5'h11)
       begin
          $display("FAILED - defparam3.5A - Defparam testmodA.ident0");
          $display("out0 = %h",out0);
          error = 1;
       end
     #1 ;
     in0 = 1;
     #1 ;
     if(out0 != 5'h4)
       begin
          $display("FAILED - defparam3.5A - Defparam testmodA.ident0");
          error = 1;
       end
     #1;
     in1 = 0;
     #1;
     if(out0 != 5'h4)	// Validate the 0 side didn't change!
       begin
          $display("FAILED - defparam3.5A - Defparam testmodA.ident0");
          error = 1;
       end
     if(out1 != 5'h6)
       begin
          $display("FAILED - defparam3.5A - Defparam testmodB.ident1");
          error = 1;
       end
     #1;
     in1 = 1;
     #1;
     if(out1 != 5'h5)
       begin
          $display("FAILED - defparam3.5A - Defparam testmodB.ident0");
          error = 1;
       end


     if(error == 0)
       $display("PASSED");
  end
endmodule // main
