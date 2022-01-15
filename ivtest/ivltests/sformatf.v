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

// Test for $sformatf system function.

module ivl_sformatf_test;
wire test_net;
assign (pull1, strong0) test_net = 1'b1;

struct packed {
    logic [15:0] high;
    logic [15:0] low;
} word;

initial begin
    string f;
    word = 32'b0101_0101_0101_0101_0101_0101_0101_0101;

    // Test constant values
    // Integers
    f = $sformatf("sformatf test 1: %b %d %o %x", 8'd120, -12, 331, 120, 97);
    if(f != "sformatf test 1: 01111000         -12 00000000513 00000078         97") begin
        $display(f);
        $display("FAILED 1");
        $finish();
    end

    // Floats
    f = $sformatf("sformatf test 2: %e %f %g", 123.45, 100e12, 100e12);
    if(f != "sformatf test 2: 1.234500e+02 100000000000000.000000 1e+14") begin
        $display(f);
        $display("FAILED 2");
        $finish();
    end

    // Strings
    f = $sformatf("sformatf test 3: %s %c", "'string test'", 97);
    if(f != "sformatf test 3: 'string test' a") begin
        $display(f);
        $display("FAILED 3");
        $finish();
    end

    // Other stuff
    f = $sformatf("sformatf test 4: %t %v %u %z", 120s, test_net, word, word);
    if(f != "sformatf test 4:                  120 Pu1 UUUU UUUU") begin
        $display(f);
        $display("FAILED 4");
        $finish();
    end

    $display("PASSED");
end
endmodule
