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


// Basic test for 'real' floating-type support in VHDL.

module vhdl_real_testbench;
  vhdl_real dut();

  initial begin
    // simply check if the assigned values are correct
    if (dut.c != 1111.222) begin
      $display("FAILED");
      $finish;
    end

    if (dut.e != 1135.022) begin
      $display("FAILED");
      $finish;
    end

    if (dut.a != 1.2) begin
      $display("FAILED");
      $finish;
    end

    if (dut.b != 32.12323) begin
      $display("FAILED");
      $finish;
    end

    if (dut.exp != 2.334e+2) begin
      $display("FAILED");
      $finish;
    end

    #10;    // wait for the no_init signal assignment
    if (dut.no_init != 33.32323) begin
      $display("FAILED");
      $finish;
    end

    $display("PASSED");
  end
endmodule
