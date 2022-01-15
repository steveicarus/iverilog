/*
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
// $Id: rop.v,v 1.2 2001/06/20 00:04:09 ka6s Exp $
// $Log: rop.v,v $
// Revision 1.2  2001/06/20 00:04:09  ka6s
// Updated the code to print out "PASSED" when appropriate.
//
// Revision 1.1  2001/06/19 13:52:13  ka6s
// Added 4 tests from Stephan Boettcher
//
//
// Test of === operator

module rop;

   reg [2:0] a;
   reg	     b;
   reg	     error;

   initial
     begin
    error = 0;
	a = 3'b 10z;
	b =  & a;
	$display(" & 3'b%b === %b", a, b);
	if (b !== 1'b0)
	begin
	   $display("FAILED");
	    error = 1;
        end
	b =  | a;
	$display(" | 3'b%b === %b", a, b);
	if (b !== 1'b1)
          begin
	   error = 1;
	   $display("FAILED");
	  end
	b =  ^ a;
	$display(" ^ 3'b%b === %b", a, b);
	if (b !== 1'bx)
          begin
	   $display("FAILED");
	   error = 1;
          end
	b = ~& a;
	$display("~& 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~| a;
	$display("~| 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~^ a;
	$display("~^ 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	a = 3'b 0xz;
	b =  & a;
	$display(" & 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  | a;
	$display(" | 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  ^ a;
	$display(" ^ 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~& a;
	$display("~& 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~| a;
	$display("~| 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~^ a;
	$display("~^ 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	a = 3'b 1xz;
	b =  & a;
	$display(" & 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  | a;
	$display(" | 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  ^ a;
	$display(" ^ 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~& a;
	$display("~& 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~| a;
	$display("~| 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~^ a;
	$display("~^ 3'b%b === %b", a, b);
	if (b !== 1'bx)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	a = 3'b 000;
	b =  & a;
	$display(" & 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  | a;
	$display(" | 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  ^ a;
	$display(" ^ 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~& a;
	$display("~& 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~| a;
	$display("~| 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~^ a;
	$display("~^ 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	a = 3'b 111;
	b =  & a;
	$display(" & 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  | a;
	$display(" | 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b =  ^ a;
        $display(" ^ 3'b%b === %b", a, b);
	if (b !== 1'b1)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~& a;
	$display("~& 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~| a;
	$display("~| 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
	b = ~^ a;
	$display("~^ 3'b%b === %b", a, b);
	if (b !== 1'b0)
	  begin
	   $display("FAILED");
	   error = 1;
	  end
        if(error === 0)
	   $display("PASSED");
     end

endmodule
