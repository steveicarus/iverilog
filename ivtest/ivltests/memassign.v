//
// Copyright (c) 1999 David Leask (david.leask@ctam.com.au)
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
//  SDW - Modified to be self checking

/*
**      The problem:
**      Reading in a series of bytes, one per clock, to create a
**      large vector which holds the bytes in a parallel form.
*/


module  demo_assign_problem;

reg     [7:0] mem_buffer [0:3];
wire    [31:0] big_word;
reg     error;
reg     [31:0] myconst;

integer i;

assign big_word[ 31: 24] = mem_buffer[0];
assign big_word[ 23: 16] = mem_buffer[1];
assign big_word[ 15:  8] = mem_buffer[2];
assign big_word[  7:  0] = mem_buffer[3];

initial
  begin
  error = 0;

  for (i = 0; i < 4; i = i+1)
    mem_buffer[i] = 0;
  #50;
  mem_buffer[0] = 8'h12;
  #50;
  myconst = 32'h12_00_00_00;
  if(big_word !== 32'h12_00_00_00)
    begin
      $display("FAILED -Memory assign - expect %h, but have %h",
                myconst,big_word);
      error = 1;
    end
  #100  ;
  mem_buffer[1] = 8'h34;
  #50;
  myconst = 32'h12_34_00_00;
  if(big_word !== 32'h12_34_00_00)
    begin
      $display("FAILED -Memory assign - expect %h, but have %h",
                 myconst,big_word);
      error = 1;
    end
  #100 ;
  mem_buffer[2] = 8'h56;
  #50;
  myconst = 32'h12_34_56_00;
  if(big_word !== 32'h12_34_56_00)
    begin
      $display("FAILED -Memory assign - expect %h, but have %h",
                 myconst,big_word);
      error = 1;
    end
  #100;
  mem_buffer[3] = 8'h78;
  #50;
  myconst = 32'h12_34_56_00;
  if(big_word !== 32'h12_34_56_78)
    begin
      $display("FAILED - Memory assign - expect %h, but have %h",
               myconst,big_word);
      error = 1;
    end
  #100;
  mem_buffer[0] = 8'hab;
  #50;
  myconst = 32'hab_34_56_00;
  if(big_word !== 32'hab_34_56_78)
    begin
      $display("FAILED - Memory assign - expect %h, but have %h",
                myconst,big_word);
      error = 1;
    end

  #100;
  if (error ===0)
     $display("PASSED");


end
endmodule
