// Copyright (c) 2015 CERN
// Maciej Suminski <maciej.suminski@cern.ch>
//
// This source code is free software; you can redistribute it
// and/or modify it in source code form under the terms of the GNU
// General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA


// Basic test for functions that work with unbounded vectors as return
// and param types.

module vhdl_unbounded_func_test();
  vhdl_unbounded_func dut();

  initial begin
    #1;   // wait for signal assignment

    if(dut.test_out1 != 'b1010100110) begin
      $display("FAILED 1");
      $finish;
    end

    if(dut.test_out2 != 'b010110) begin
      $display("FAILED 2");
      $finish;
    end

    if(dut.neg_test_out1 != ~dut.test_out1) begin
      $display("FAILED 3");
      $finish;
    end

    if(dut.neg_test_out2 != ~dut.test_out2) begin
      $display("FAILED 4");
      $finish;
    end

    $display("PASSED");
  end
endmodule

