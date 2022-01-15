//
// Copyright (c) 1999 Steve Wilson (stevew@home.com)
// Based on code contributed by Peter Monta (pmonta@imedia.com)
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
//	SDW - Validate XOR op using non-blocking assignment
//

module main;
reg [7:0] a;
reg b;
reg c;
reg error;

initial
  begin
   #1;
    error = 0;
     for(a = 0; a <= 8'h1; a = a + 1)
       begin
          b = 0;
          #1 ;
          if(a)
            begin
              if(!c)
                begin
                  $display("FAILED - XOR a=%b,b=%b,c=%b",a,b,c);
                  error = 1;
                end
            end
          if(!a)
            begin
              if(c)
                begin
                  $display("FAILED - XOR a=%b,b=%b,c=%b",a,b,c);
                  error = 1;
                end
            end
          b = 1;
          #1 ;
          if(!a)
            begin
              if(!c)
                begin
                  $display("FAILED - XOR a=%b,b=%b,c=%b",a,b,c);
                  error = 1;
                end
            end
          if(a)
            begin
              if(c)
                begin
                  $display("FAILED - XOR a=%b,b=%b,c=%b",a,b,c);
                  error = 1;
                end
            end
       end
    if(!error)
       $display("PASSED");
  end

always @(a or b)
   c <= a ^ b;
endmodule
