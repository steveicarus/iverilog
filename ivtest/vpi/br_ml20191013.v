/**********************************************************************
 * $pow example -- Verilog HDL test bench.
 *
 * Verilog test bench to test the $pow PLI application.
 *
 * For the book, "The Verilog PLI Handbook" by Stuart Sutherland
 *  Copyright 1999 & 2001, Kluwer Academic Publishers, Norwell, MA, USA
 *   Contact: www.wkap.il
 *  Example copyright 1998, Sutherland HDL Inc, Portland, Oregon, USA
 *   Contact: www.sutherland-hdl.com
 *********************************************************************/
`timescale 1ns / 1ns
module test;

  reg [31:0]  result;
  reg         a, b;

  initial
    begin
      $display("Start simulation pow_test.v");
      a = 1;
      b = 0;

      /* Test $pow with valid values */
      #1 $display("$pow(2,3) returns %d", $pow(2,3));
      #1 result = $pow(a,b);
      #1 $display("$pow(a,b) returns %d (a=%d b=%d)", result, a, b);
      #1 $finish(0);
    end

endmodule
/*********************************************************************/
