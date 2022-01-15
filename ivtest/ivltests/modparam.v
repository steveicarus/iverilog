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
//  SDW - Validate parameter passing override in module declaration.
//
// Build a single line of storage - Note it's
//

module reg32 (clk,we, din, dout);

parameter WIDTH=32;

input			we;
input			clk;
input [WIDTH-1:0]	din;

output [WIDTH-1:0]	dout;

reg [WIDTH-1:0] store;

always @(posedge clk)
  if(we)
    store <= din;

assign dout = store ;

endmodule

module memory(clk, we, addr, din,  dout);

parameter WIDTH=8;

input			clk;
input			we;
input [1:0]		addr;
input [WIDTH-1:0]	din;

output [WIDTH-1:0]	dout;
reg    [WIDTH-1:0]	dout;

wire    [WIDTH-1:0]	dout0,dout1,dout2,dout3;
reg			we0,we1,we2,we3;

reg32 #(WIDTH) reg0 (.clk(clk),.we(we0),.din(din[WIDTH-1:0]),
                     .dout(dout0[WIDTH-1:0]));
reg32 #(WIDTH) reg1 (.clk(clk),.we(we1),.din(din[WIDTH-1:0]),
                     .dout(dout1[WIDTH-1:0]));
reg32 #(WIDTH) reg2 (.clk(clk),.we(we2),.din(din[WIDTH-1:0]),
                     .dout(dout2[WIDTH-1:0]));
reg32 #(WIDTH) reg3 (.clk(clk),.we(we3),.din(din[WIDTH-1:0]),
                     .dout(dout3[WIDTH-1:0]));

//
// Build we decode
//
always @(addr or we)
   case (addr)
      2'b00: begin
               we0 = we;
               we1 = 0;
               we2 = 0;
               we3 = 0;
             end
      2'b01: begin
               we0 = 0;
               we1 = we;
               we2 = 0;
               we3 = 0;
             end
      2'b10: begin
               we0 = 0;
               we1 = 0;
               we2 = we;
               we3 = 0;
             end
      2'b11: begin
               we0 = 0;
               we1 = 0;
               we2 = 0;
               we3 = we;
             end
   endcase

//
// Connect dout to register output
//
always @(addr or dout0 or dout1 or dout2 or dout3)
   case (addr)
      2'b00: dout = dout0;
      2'b01: dout = dout1;
      2'b10: dout = dout2;
      2'b11: dout = dout3;
   endcase

endmodule

module top;

parameter WIDTH=8;
reg			clk;
reg			we;
reg    [1:0]		addr;
reg   [WIDTH-1:0]		din;
reg			error;
wire  [WIDTH-1:0]		dout;

memory mem (clk, we, addr, din,  dout);

initial
  begin
//    $dumpfile("test.vcd");
//    $dumpvars(0,top.mem.reg0);

    clk = 0;
    error =0;
    #3;
    we = 1;
    addr = 0;
    din = 32'b0_00;
    #10;
    addr = 1;
    din = 32'h1;
    #10;
    addr = 2;
    din = 32'h2;
    #10;
    addr = 3;
    din = 32'h3;
    #10;
    we = 0;
    addr = 0;
    #1;
    if(dout[7:0] !== 8'h00)
      begin
         $display("FAILED - Ram[0] not 0, is %h",dout[7:0]);
         error = 1;
      end
    if(error == 0)
      $display("PASSED");
    $finish ;
  end

always #(5) clk = ~clk;
endmodule
