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
//
// SDW: Module instantiation with non-ordered port assignment
//
// D: Same as sdw_inst1 except using .net(net) port convention.
//

module test_mod (reset,clka,out);
input reset;
input clka;
output out;

reg out;

always @(posedge clka or posedge reset)
  if(reset)
     out = 0;
  else
     out = ~out;

endmodule

module main();

reg reset,clk_0,clk_1;
wire out_0,out_1;

test_mod module_1 (.reset(reset),.clka(clk_0),.out(out_0));
test_mod module_2 (.reset(reset),.clka(clk_1),.out(out_1));

initial
  begin
    clk_0 = 0;
    clk_1 = 0;
    #1 reset = 1;
    # 2;
    $display("time %d r=%b, c0=%b, c1=%b, o0=%b,o1=%b\n",$time,reset,clk_0,
              clk_1,out_0,out_1);
    // Validate that both out_0 and out_1 are reset
    if(out_0)
      begin
        $display("FAILED - out_0 not reset\n");
        $finish ;
      end

    if(out_1)
      begin
        $display("FAILED - out_1 not reset\n");
        $finish ;
      end
     reset = 0;
    $display("time %d r=%b, c0=%b, c1=%b, o0=%b,o1=%b\n",$time,reset,clk_0,
              clk_1,out_0,out_1);
     # 2;
    clk_0 = 1;
    # 2; // Wait so we don't worry about races.
    $display("time %d r=%b, c0=%b, c1=%b, o0=%b,o1=%b\n",$time,reset,clk_0,
              clk_1,out_0,out_1);
    if(!out_0)
      begin
        $display("FAILED - out_0 didn't set on clk_0\n");
        $finish ;
      end

    if(out_1)
      begin
        $display("FAILED - out_1 set on wrong clk!\n");
        $finish ;
      end

    clk_1 = 1;
    # 2; // Wait so we don't worry about races.
    $display("time %d r=%b, c0=%b, c1=%b, o0=%b,o1=%b\n",$time,reset,clk_0,
              clk_1,out_0,out_1);
    if(!out_1)
      begin
        $display("FAILED - out_1 didn't set on clk_1\n");
        $finish ;
      end

    if(!out_0)
      begin
        $display("FAILED - out_0 changed due to clk_0\n");
        $finish ;
      end


    $display("PASSED\n");
    $finish ;
  end
endmodule
