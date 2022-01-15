//
// Copyright (c) 2001  Ed Schwartz (schwartz@r11.ricoh.com)
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
//  SDW - Verify PR142 - Added something to print PASSED..

module testit;

   reg clk;
   reg [2:0] cnt;

   always
      begin
         # 50 clk = ~clk;
      end // always begin

   task idle;
      input [15:0] waitcnt;

   begin: idletask
   // begin
      integer i;
      for (i=0; i < waitcnt; i = i + 1)
         begin
            @ (posedge clk);

         end // for (i=0; i < waitcnt; i = i + 1)

   end
   endtask // idle

   initial begin
       clk = 0;
       cnt = 0;
       $display ("One");
       cnt = cnt + 1;
       idle(3);
       cnt = cnt + 1;
       $display ("Two");
       if(cnt === 2)
         $display("PASSED");
       else
         $display("FAILED");
       $finish;
   end

endmodule
