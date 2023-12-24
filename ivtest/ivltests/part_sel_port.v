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


// Test part selection in multidimensional packed ports assignment.

module mod_test(output wire logic[1:8][7:0] out);
    assign out = "testTEST";
endmodule

module mod_test2(output wire logic[1:8][7:0] out);
    assign out = "abcdefgh";
endmodule

module mod_main;
logic[1:16][7:0] test_string;
mod_test dut(test_string[1:8]);
mod_test2 dut2(test_string[9:16]);

initial begin
    #0
    if(test_string !== "testTESTabcdefgh") begin
        $display("FAILED");
        $finish();
    end

    $display("PASSED");
end

endmodule
