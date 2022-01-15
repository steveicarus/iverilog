//
// Copyright (c) 2002 Stephen Williams
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//

module main;

   reg ena, wea;
   reg enb, web;

   reg clk;
   reg out = 0;

   always @(posedge clk) begin
      if ((ena == 1) && (wea == 1) &&
	  (enb == 1) && (web == 1))
	out <= 1;
   end

   initial begin
      clk = 0;
      ena = 0;
      enb = 0;
      wea = 0;
      web = 0;

      $monitor("clk=%b: ena=%b, enb=%b, wea=%b, web=%b --> out=%b",
	       clk, ena, enb, wea, web, out);

      #1 clk = 1;
      #1 clk = 0;

      ena = 1;
      enb = 1;

      #1 clk = 1;
      #1 clk = 0;

      wea = 1;
      web = 1;

      #1 clk = 1;
      #1 clk = 0;
   end // initial begin

endmodule // main
