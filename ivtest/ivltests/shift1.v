/*
 * Copyright (c) 2001 Stephen Rowland
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
module dummy;

reg  [7:0] decode_vec;
wire [7:0] data1;
wire [7:0] data2;

// icarus cant handle this statement
assign data1 = (decode_vec[8'h02>>1] ) ? 8'h55 : 8'h00;

assign data2 = (decode_vec[8'h01   ] ) ? 8'h55 : 8'h00;

initial
begin
#0;
$monitor("%h %h %h", decode_vec, data1, data2);
decode_vec = 8'h02;
#10;
decode_vec = 8'h80;
#10;
decode_vec = 8'h02;
#10;
$finish(0);
end

endmodule
