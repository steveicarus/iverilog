//
// Copyright (c) 1999 Steven Wilson (stevew@home.com)
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
//
// SDW - Verify task basic function with no values passed.
//
// D: Task validation. Notice that there are no values passed to this task.
//
//

module main();

reg [3:0] global_reg;

task dec_glob;
begin
  global_reg = global_reg -1;
end
endtask

initial
begin
  global_reg = 2;
  dec_glob;

  if(global_reg != 1)
    begin
      $display("FAILED - task didn't modify global_reg\n");
      $finish ;
    end
  $display("PASSED\n");
  $finish ;
end

endmodule
