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
//  SDW - Validate task calling a function


module main;

reg var1,var2;
reg in1;
reg error;

function foo ;
  input in1 ;
  foo = in1 ;
endfunction

task my_task ;
  input in1,in2;
  output out1,out2;
  begin
    out1 = foo(~foo(in1)) ;
    out2 = foo(in2) ;
  end
endtask

initial
  begin
    error = 0;
    my_task(1'b1,1'b0,var1,var2);
    if(~(~var1 & ~var2))
      begin
        $display("FAILED - task 3.14E task calling a function (1)");
         error = 1;
      end

    in1 = 0;
    my_task(~in1,~in1,var1,var2);
    if(~(~var1 & var2))
      begin
         $display("FAILED - task 3.14E task calling a function(2)");
         error = 1;
      end

    in1 = 0;
    my_task(in1,in1,var1,var2);
    if(~(var1 & ~var2))
      begin
         $display("FAILED - task 3.14E task calling a function(3)");
         error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule // main
