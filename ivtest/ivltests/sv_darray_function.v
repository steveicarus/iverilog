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


// Test for dynamic arrays used as the function parameters and return type.

module sv_darray_function();
typedef logic[7:0] byte_array [];
typedef logic[3*8-1:0] byte_vector;

function byte_array inc_array(byte_array inp);
    byte_array tmp;
    tmp = new[$size(inp)];

    for(int i = 0; i < $size(inp); ++i)
    begin
        tmp[i] = inp[i] + 1;
    end

    return tmp;
endfunction

initial begin
    byte_array a, b;
    byte_vector c;

    a = new[3];
    a[0] = 10;
    a[1] = 11;
    a[2] = 12;
    b = inc_array(a);

    if($size(a) != 3 || a[0] !== 10 || a[1] !== 11 || a[2] !== 12) begin
        $display("FAILED 1");
        $finish();
    end

    if($size(b) != 3 || b[0] !== 11 || b[1] !== 12 || b[2] !== 13) begin
        $display("FAILED 2");
        $finish();
    end

    // Cast dynamic array returned by function to logic vector
    c = byte_vector'(inc_array(b));
    if(c !== 24'h0c0d0e) begin
        $display("FAILED 3");
        $finish();
    end

    $display("PASSED");
end
endmodule
