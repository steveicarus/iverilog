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
//  SDW - Validate declared wire and implicit wires displayed.
//


// This circuit has 3 i/os and 3 implicit wires. Both should be
// present in vcd file??
module xorckt (out,in0,in1);
input in0;
input in1;

wire junk;

nand  #1 na1 (na1_out,in0,in1);
nand  #1 na2 (na2_out,in0,na1_out);
nand  #1 na3 (na3_out,in1,na1_out);
nand  #1 na4 (out,na2_out,na3_out);

assign junk = in0;

endmodule

module main;

wire xout;
reg i1,i2;

xorckt myckt (.out(xout),.in0(i1),.in1(i2));

initial
  begin
    $dumpfile("work/test.vcd");
    $dumpvars(0,main.myckt);
    i1 = 1'b0;
    i2 = 1'b0;
    #5;
    $display("%b xor %b = %b",i1,i2,xout);
    i1 = 1'b1;
    i2 = 1'b0;
    #5;
    $display("%b xor %b = %b",i1,i2,xout);
    i1 = 1'b1;
    i2 = 1'b1;
    #5;
    $display("%b xor %b = %b",i1,i2,xout);
    i1 = 1'b0;
    i2 = 1'b1;
    #5 ;
    $display("%b xor %b = %b",i1,i2,xout);
  end

endmodule // main
