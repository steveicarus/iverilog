/*
 * integer4ge - a verilog test for integer greater-or-equal conditional >=
 *
 * Copyright (C) 2000 Steve Wilson stevew@home.com
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
`timescale 100s/1s
module test;

reg [3:0] result;
reg  error;
integer    num1;
wire [3:0] result1;

assign result1 = 1 + (num1 /4);

initial
  begin
    error = 0;
    num1 = 32'h24 ;
    result = 1 + (num1 / 4);
    #1;
    if(result !== 4'ha)
      begin
        $display("FAILED - division didn't work s/b A, is %h",result);
        error = 1;
      end
    if(result1 !== 4'ha)
      begin
        $display("FAILED - assign division didn't work s/b A, is %h",result1);
        error = 1;
      end
    if(error == 0)
      $display("PASSED");
  end

endmodule
