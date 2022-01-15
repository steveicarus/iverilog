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


// Test for casting a dynamic array to vector type
// using Icarus specific VPI functions

module sv_cast_string();
    bit [7:0] darr [];
    bit [63:0] darr_64 [];
    bit [7*8 - 1:0] arr;
    bit [127:0] arr_128;

initial begin
    darr_64 = new[2];
    darr_64[0] = "ABCDEFGH";
    darr_64[1] = "IJKLMNOP";

    darr = new[7];
    // Set darr to '{"a","b","c","d","e","f","g"}
    foreach(darr[i])
        darr[i] = "a" + i;

    // Casting dynamic array to vector
    $ivl_darray_method$to_vec(darr, arr);
    if(arr !== "abcdefg") begin
        $display("FAILED 1");
        $finish();
    end

    $ivl_darray_method$to_vec(darr_64, arr_128);
    if(arr_128 !== "ABCDEFGHIJKLMNOP") begin
        $display("FAILED 2");
        $finish();
    end

    // Reset the stored data to perform reverse casting test
    arr = "0123456";
    arr_128 = "cafedeadbeefc0de";

    // Casting vector to dynamic array
    $ivl_darray_method$from_vec(darr, arr);
    foreach(darr[i]) begin
        if(darr[i] != "0" + i) begin
            $display("FAILED 3");
            $finish();
        end
    end

    $ivl_darray_method$from_vec(darr_64, arr_128);
    if(darr_64[0] !== "cafedead" || darr_64[1] !== "beefc0de") begin
        $display("FAILED 4");
        $finish();
    end

    $display("PASSED");
end
endmodule

