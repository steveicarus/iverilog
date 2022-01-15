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
//  SDW - Validate always fork : id block_decl  parallel_statements join



module main ;

reg [3:0] value1,value2,value3;

always fork : fork_id
         reg [3:0] value4 ;
         #5 begin
              value4 = 0;
              value1 = value4 + 1 ;
            end
         #10 value1 = value4 + 2;
       join

initial
  begin
    value1 = 0;
    value2 = 0;
    #4 ;
    if(value1 != 0)
      begin
         $display("FAILED - 3.1.12C always fork : id block_decl statements join (0)");
          value2 = 1;
      end
    #2 ;
    if(value1 != 1)
      begin
         $display("FAILED - 3.1.12C always fork : id block_decl statements join (1)");
          value2 = 1;
      end
    #5 ;
    if(value1 != 2)
      begin
         $display("FAILED - 3.1.12C always fork : id block_decl statements join (2)");
          value2 = 1;
      end
    #5 ;
    if(value1 != 1)
      begin
         $display("FAILED - 3.1.12C always fork : id block_decl statements join (3)");
          value2 = 1;
      end
    if(value2 == 0) $display("PASSED");
    $finish ;
  end
endmodule
