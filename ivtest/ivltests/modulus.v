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
//  SDW - Validate Modulus operator

module top;

reg [7:0] a,b;
wire [7:0] wa,wb;
reg  [7:0] result;
wire [7:0] wresult;
reg  [15:0] work;
reg  error;

assign wa = a;
assign wresult = work % a;

always @ (work or wa)
  result = work % a;

initial
  begin
    error = 0;
    /* Try mod div by 0 */
    #1;
    a = 0;
    work = 16'd1235;
    #1;
    if(wresult !== 8'hxx)
      begin
         $display("FAILED - wire 1235 mod 0: wresult = %h",wresult);
         error =1;
      end
    if(result !== 8'hxx)
      begin
         $display("FAILED - reg 1235 mod 0: result = %h",result);
         error =1;
      end
    #1;
    a = 8'd10;
    #1;
    if(wresult !== 8'h05)
      begin
         $display("FAILED - wire 1235 mod 10: wresult = %h",wresult);
         error =1;
      end
    if(result !== 8'h05)
      begin
         $display("FAILED - reg 1235 mod 10: result = %h",result);
         error =1;
      end

    #1;
    a = 8'b0000_x001;
    #1;
    if(wresult !== 8'bxxxx_xxxx)
      begin
         $display("FAILED - wire 1235 mod 10: wresult = %h",wresult);
         error =1;
      end
    if(result !== 8'bxxxx_xxxx)
      begin
         $display("FAILED - reg 1235 mod 10: result = %h",result);
         error =1;
      end


    if(error == 0)
     $display("PASSED");

  end

endmodule
