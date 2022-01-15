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


// Resize function test

module vhdl_resize_test;
logic signed [7:0] in;
logic signed [15:0] out;
vhdl_resize dut(in, out);

initial begin
    in = -120;
    #0;
    if(out !== -115) begin
        $display("FAILED 1: out = %d", out);
        $finish();
    end

    in = 120;
    #0;
    if(out !== 125) begin
        $display("FAILED 2: out = %d", out);
        $finish();
    end

    $display("PASSED");
end

endmodule
