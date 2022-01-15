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
//  SDW - Validate task with internal delay


module main;

reg var1,var2;
reg in1;
reg error;

task my_task ;
  input in1;
  output out1;
    out1 = #10 in1;
endtask

initial
  begin
     var1 = 0;
     error = 0;
     fork
         my_task(1'b1,var1);
         begin
           if(var1 != 1'b0)
             begin
                $display("FAILED - task3.14F Task with internal delay(1)");
                error = 1;
             end
           #20;
           if(var1 != 1'b1)
             begin
                $display("FAILED - task3.14F Task with internal delay(2)");
                error = 1;
             end
         end
     join
     if(error == 0)
       $display("PASSED");
  end

endmodule // main
