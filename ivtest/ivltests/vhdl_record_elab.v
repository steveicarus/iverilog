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


// Tests initialization of records with aggregate expressions.
// (based on the vhdl_struct_array test)

module vhdl_record_elab_test;
  reg [15:0] in;
  wire [15:0] out;

  vhdl_record_elab dut(
            .o_high1(out[15:12]), .o_low1(out[11:8]),
            .o_high0(out[7:4]),   .o_low0(out[3:0]),

            .i_high1(in[15:12]), .i_low1(in[11:8]),
            .i_high0(in[7:4]),   .i_low0(in[3:0]));

  initial begin
    for (in = 0 ; in < 256 ; in = in+1) begin
        #1 if (in !== out[15:0]) begin
            $display("FAILED -- out=%h, in=%h", out, in);
            $finish;
        end
    end

    if (dut.dword_a[0].low !== 4'b0110 || dut.dword_a[0].high !== 4'b1001 ||
        dut.dword_a[1].low !== 4'b0011 || dut.dword_a[1].high !== 4'b1100)
    begin
        $display("FAILED 2");
        $finish;
    end

    $display("PASSED");
  end
endmodule

