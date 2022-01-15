/* talu - a verilog test,
 * illustrating problems I had in fragments of an ALU from an 8-bit micro
 */

module talu;
   reg error;

   reg [7:0] a;
   reg [7:0] b;
   reg cin;
   reg [1:0] op;

   wire cout;
   wire [7:0] aluout;

   alu alu_m(a, b, cin, op, aluout, cout);

   initial begin
       error = 0;

       // add
       op='b00; cin='b0; a='h0; b='h0;
       #2 if({cout, aluout} != 9'h000) begin
	   $display($time, " FAILED %b  %b %h %h  %b %h", op, cin, a, b, cout, aluout);
	   error = 1;
       end

       // add1
       op='b01; cin='b0; a='h01; b='h01;
       #2 if({cout, aluout} != 9'h103) begin
	   $display($time, " FAILED %b  %b %h %h  %b %h", op, cin, a, b, cout, aluout);
	   error = 1;
       end

       // and
       op='b10; cin='b0; a='h16; b='h0F;
       #2 if({cout, aluout} != 9'h006) begin
	   $display($time, " FAILED %b  %b %h %h  %b %h", op, cin, a, b, cout, aluout);
	   error = 1;
       end
       op='b10; cin='b0; a='h28; b='hF7;
       #2 if({cout, aluout} != 9'h020) begin
	   $display($time, " FAILED %b  %b %h %h  %b %h", op, cin, a, b, cout, aluout);
	   error = 1;
       end

       // genbit
       op='b11; cin='b0; a='h00; b='h03;
       #2 if({cout, aluout} != 9'h008) begin
	   $display($time, " FAILED %b  %b %h %h  %b %h", op, cin, a, b, cout, aluout);
	   error = 1;
       end
       op='b11; cin='b0; a='h00; b='h00;
       #2 if({cout, aluout} != 9'h001) begin
	   $display($time, " FAILED %b  %b %h %h  %b %h", op, cin, a, b, cout, aluout);
	   error = 1;
       end
       /* tests are incomplete - doesn't compile yet on ivl */


       if(error == 0)
	   $display("PASSED");
       $finish;

   end

endmodule

/*
 * fragments of an ALU from an 8-bit micro
 */

module alu(Aval, Bval, cin, op, ALUout, cout);
   input [7:0]	Aval;
   input [7:0]	Bval;
   input	cin;
   input [1:0]	op;
   output	cout;
   output [7:0]	ALUout;

   reg		cout;
   reg [7:0]	ALUout;

   always @(Aval or Bval or cin or op) begin
      case(op)
	2'b00 : {cout, ALUout} = Aval + Bval;
	2'b10 : {cout, ALUout} = {1'b0, Aval & Bval};

// C++ compilation troubles with both of these:
	2'b01 : {cout, ALUout} = 9'h100 ^ (Aval + Bval + 9'h001);
	2'b11 : {cout, ALUout} = {1'b0, 8'b1 << Bval};

//	2'b01 : {cout, ALUout} = 9'h000;
//	2'b11 : {cout, ALUout} = 9'h000;
      endcase
   end // always @ (Aval or Bval or cin or op)

endmodule

/* Copyright (C) 1999 Stephen G. Tell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */
