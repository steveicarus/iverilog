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
//  SDW - Validate assign procedural assign {ident1,ident0}  = expr;

module main ;


reg a,b,c,d;
reg control;
reg clock;
reg error;

always @(posedge clock)
  {a,b,c,d} = 4'h3;

always @(control)
  if(control)
    assign {a,b,c,d}  = 4'h2;
  else
    deassign {a,b,c,d}  ;


// Setup a clock generator.
always begin
         #2;
         clock = ~clock;
       end

initial
      begin
        clock = 0;
        error = 0;
        # 3;
        if({a,b,c,d} !== 3)
          begin
            $display("FAILED - assign3.2D - procedural assignment(1)");
            error = 1;
          end
        # 2;
        control = 1;
        # 1;
        if({a,b,c,d} !== 2)
          begin
            $display("FAILED - assign3.2D - procedural assignment(2)");
            error = 1;
          end
        # 3 ;
        control = 0;
        # 2;
        if({a,b,c,d} !== 3)
          begin
            $display("FAILED - assign3.2D - procedural assignment(3)");
            error = 1;
          end

        if(error == 0) $display ("PASSED");
        $finish ;

      end

endmodule
