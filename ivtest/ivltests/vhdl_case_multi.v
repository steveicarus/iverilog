// Copyright (c) 2014 CERN
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

// Test for multiple choices in case alternative statements.

module vhdl_case_multi_test;
  reg [2:0] test_vec;
  reg parity;
  vhdl_case_multi dut(test_vec, parity);

  initial begin
    // Execute both paths
    test_vec = 'b101;
    #1;
    if(parity !== 1'b0) begin
        $display("FAILED 1");
        $finish();
    end

    test_vec = 'b001;
    #1;
    if(parity !== 1'b1) begin
        $display("FAILED 2");
        $finish();
    end

    $display("PASSED");
  end
endmodule

