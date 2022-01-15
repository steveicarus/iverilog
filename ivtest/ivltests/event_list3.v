/*
 * Copyright (c) 2001 Juergen Urban
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
module sensitivity_list ();
parameter ii = 4;
reg CLK;
reg A;
reg [ii-1:0] B,C;
initial
begin
  #30;
  C <= {ii{1'b0}};
  #60;
  $finish(0);
end

always
begin
  CLK = 1'b0;
  #10;
  CLK = 1'b1;
  #10;
  $display ($time);
end

always @(A or C) begin
    A = 1'b0;
    $display ("combinatorial process ", A, " time:",$time);
    A = 1'b1;
    $display ("combinatorial process ", A, " time:",$time);
    B = A+C;
  end
endmodule
