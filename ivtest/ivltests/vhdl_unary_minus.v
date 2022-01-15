// Copyright (c) 2016 CERN
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


// Unary minus operator test

module vhdl_unary_minus_test;
logic signed [7:0] data_in;
logic signed [7:0] data_out;
logic clk = 1'b0;

vhdl_unary_minus dut(data_in, clk, data_out);

always #10 clk = ~clk;

initial begin
    #5;

    data_in = -12;
    #20;
    if(data_out !== 12) begin
        $display("FAILED 1");
        $finish();
    end

    data_in = 33;
    #20;
    if(data_out !== -33) begin
        $display("FAILED 2");
        $finish();
    end

    $display("PASSED");
    $finish();
end

endmodule
