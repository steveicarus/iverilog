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


// Tests signal initializers.

module vhdl_init_testbench;
  vhdl_init dut();

  initial begin
    // Simply check if the assigned values are correct
    if (dut.a !== 'b11101001) begin
      $display("FAILED #1: expected 11101001, got %b", dut.a);
      $finish;
    end

    if (dut.b !== 'b1010) begin
      $display("FAILED #2: expected 1010, got %b", dut.b);
      $finish;
    end

    if (dut.c !== 'b1000) begin
      $display("FAILED #3: expected 1000, got %b", dut.c);
      $finish;
    end

    $display("PASSED");
  end
endmodule
