/*
 * Copyright (c) 2000 Peter monta (pmonta@pacbell.net)
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
// Reworked by SDW to be self checking
module main;

  reg [7:0] x;
  reg [7:0] y;
  reg [2:0] i;	// Was a wire..
  reg error;

  initial begin
    #5;
    x[i] <= #1 0;
    y[i] = 0;
  end

initial
  begin
    error = 0;
    #1;
    i = 1;
    #7;
    if(x[i] !== 1'b0)
       begin
         $display("FAILED - x[1] != 0");
         error = 1;
       end
    if(y[i] !== 1'b0)
       begin
         $display("FAILED - y[1] != 0");
         error = 1;
       end
    if(error === 0)
       $display("PASSED");
  end

endmodule
