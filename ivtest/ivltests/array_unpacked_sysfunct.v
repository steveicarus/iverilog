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


// Test for array querying functions for unpacked arrays
// (IEEE Std 1800-2012 7.11)

module array_unpacked_sysfunct();
    bit [7:0] bit_darray [];
    //bit bit_darray [];    // not available yet
    bit [7:0] array [2:4];
    bit [7:0] reverse_array[5:3];
    bit [2:8] packed_array;
    bit [4:1] reverse_packed_array;

initial begin
    string test_msg = "13 characters";
    bit_darray = new[5];

    if($left(bit_darray) != 0 || $right(bit_darray) != 4) begin
        $display("FAILED 1");
        $finish();
    end

    if($left(array) != 2 || $right(array) != 4) begin
        $display("FAILED 2");
        $finish();
    end

    if($left(reverse_array) != 5 || $right(reverse_array) != 3) begin
        $display("FAILED 3");
        $finish();
    end

    if($left(test_msg) != 0 || $right(test_msg) != 12) begin
        $display("FAILED 4");
        $finish();
    end

    if($left(packed_array) != 2 || $right(packed_array) != 8) begin
        $display("FAILED 5");
        $finish();
    end

    if($left(reverse_packed_array) != 4 || $right(reverse_packed_array) != 1) begin
        $display("FAILED 6");
        $finish();
    end

    $display("PASSED");
end

endmodule
