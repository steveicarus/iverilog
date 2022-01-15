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


// Test different ways of accessing a 2D packed array.

module array_packed_2d();

reg [1:4][7:0] vec;
reg [4:1][7:0] vec2;
integer i;

initial begin
// test 1: assign using variable index
for(i = 1; i <= 4; i = i + 1)
    vec[i] = i * 2;

// display whole vector
$display("%h", vec);

// $display using variable index
for(i = 1; i <= 4; i = i + 1)
    $display(vec[i]);

// $display using constant index
$display(vec[1]);
$display(vec[2]);
$display(vec[3]);
$display(vec[4]);


// test 2: assign using a constant index
vec[1] = 2;
vec[2] = 4;
vec[3] = 6;
vec[4] = 8;

// display whole vector
$display("%h", vec);

// $display using variable index
for(i = 1; i <= 4; i = i + 1)
    $display(vec[i]);

// $display using constant index
$display(vec[1]);
$display(vec[2]);
$display(vec[3]);
$display(vec[4]);

//////////////////////////////////////////

// test 1: assign using variable index
for(i = 1; i <= 4; i = i + 1)
    vec2[i] = i * 2;

// display whole vector
$display("%h", vec2);

// $display using variable index
for(i = 1; i <= 4; i = i + 1)
    $display(vec2[i]);

// $display using constant index
$display(vec2[1]);
$display(vec2[2]);
$display(vec2[3]);
$display(vec2[4]);


// test 2: assign using a constant index
vec2[1] = 2;
vec2[2] = 4;
vec2[3] = 6;
vec2[4] = 8;

// display whole vector
$display("%h", vec2);

// $display using variable index
for(i = 1; i <= 4; i = i + 1)
    $display(vec2[i]);

// $display using constant index
$display(vec2[1]);
$display(vec2[2]);
$display(vec2[3]);
$display(vec2[4]);
end

endmodule
