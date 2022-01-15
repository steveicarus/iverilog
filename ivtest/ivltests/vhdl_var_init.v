// Copyright (c) 2015 CERN
// @author Maciej Suminski <maciej.suminski@cern.ch>
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


// Test for variable initialization.

module vhdl_var_init_test;
logic init;
logic [7:0] slv;
bit b;
int i;
vhdl_var_init dut(init, slv, b, i);

initial begin
    init = 0;
    #1 init = 1;
    #1;

    if(slv !== 8'b01000010) begin
        $display("FAILED 1");
        $finish();
    end

    if(b !== false) begin
        $display("FAILED 2");
        $finish();
    end

    if(i !== 42) begin
        $display("FAILED 3");
        $finish();
    end

    $display("PASSED");
end

endmodule
