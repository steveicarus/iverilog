/*
 * Copyright (c) 2002 Michael Ruff (mruff @chiaro.com)
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

module top;

   reg [1:0] \bot.m [1:0];

   bot bot();

   initial begin
      #1;
      \bot.m [0] = 2'b11;
      #1;
      $display("\\bot.m [0] == %b", \bot.m [0]);
      $display("bot.m[0] == %b", bot.m[0] );

      if (\bot.m [0] !== 2'b11) begin
	 $display("FAILED -- \\bot.m [0] !== 2'b11");
	 $finish;
      end

      if (bot.m[0] !== 2'b00) begin
	 $display("FAILED -- bot.m[0] !== 2'b00");
	 $finish;
      end

      $display("PASSED");
   end
endmodule // top

module bot;

   reg [1:0] m[1:0];

   initial begin
      m[0] = 2'b00;
   end
endmodule
