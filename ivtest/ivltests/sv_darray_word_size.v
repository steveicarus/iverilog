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


// Tests if dynamic array words are of appropriate size.

module sv_cast_string();
    bit [6:1] darr [];
    bit [63:0] darr_64 [];
    logic [4:10] darr_rev [];

initial begin
    darr = new[4];
    darr_64 = new[8];
    darr_rev = new[3];

    if($size(darr[0]) != 6 || $size(darr_64[2]) != 64 || $size(darr_rev[1]) != 7 ||
        $size(darr) != 4 || $size(darr_64) != 8 || $size(darr_rev) != 3)
    begin
        $display("FAILED");
        $finish();
    end

    darr[0] = 6'b110011;
    darr[1] = 6'b000011;
    darr[2] = darr[0] + darr[1];

    darr_64[0] = 64'hcafe0000dead0000;
    darr_64[1] = 64'h0000bad00000d00d;
    darr_64[2] = darr_64[0] + darr_64[1];

    darr_rev[0] = 7'b1111000;
    darr_rev[1] = 7'b0000011;
    darr_rev[2] = darr_rev[0] + darr_rev[1];

    if(darr[2] !== 6'b110110 || darr_64[2] !== 64'hcafebad0deadd00d ||
        darr_rev[2] !== 7'b1111011)
    begin
        $display("FAILED");
        $finish();
    end

    $display("PASSED");
end
endmodule

