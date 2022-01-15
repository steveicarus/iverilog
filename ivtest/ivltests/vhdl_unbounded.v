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


// Basic test for the unbounded arrays in VHDL.

module vhdl_unbounded_array_test;
  vhdl_unbounded_array dut();

  initial begin
    #1;   // wait for signal assignment

    if(dut.sig_logic != 'b01010101) begin
      $display("FAILED 1");
      $finish;
    end

    if(dut.sig_integer[2] != 1) begin
      $display("FAILED 2");
      $finish;
    end

    if(dut.sig_real[1] != 2.5) begin
      $display("FAILED 3");
      $finish;
    end

    $display("PASSED");
  end
endmodule
