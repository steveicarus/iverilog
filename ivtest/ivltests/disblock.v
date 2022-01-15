//
// Copyright (c) 2002 Philip Blundell <pb@nexus.co.uk>
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
// SDW: Disable within named block.
//

module m();

initial
  begin
      #10;
      $display("FAILED");
      $finish;
  end

task t;
  begin
    begin:wait_loop
       #1;
       while(1) begin
          #1;
	  disable wait_loop;
       end		// while(1)
    end			// wait_loop
  end
endtask

initial begin
  t;
  $display("PASSED");
  $finish;
end

endmodule
