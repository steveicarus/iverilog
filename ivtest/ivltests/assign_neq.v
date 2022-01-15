/*
 * Copyright (c) 2000 Steven Wilson (stevew@homeaddress.org)
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

/*
 *  This test looks for != operation in a continuous assignment.
 */
module test;

integer a;
integer b;
wire  result;
integer error;

assign result = (a != b);

initial
  begin
    a =  0;
    b = 0;
    error = 0;
    #5 ;
    if( result === 1'b1)
       error =1;
    a = 1;
    #5;
    if( result === 1'b0)
       error =1;

    b = 1;
    #5 ;
    if( result === 1'b1)
       error =1;

    a = 1002;
    b = 1001;
    #5 ;
    if( result === 1'b0)
       error =1;
    a = 1001;
    #5 ;
    if( result === 1'b1)
       error =1;

    if(error === 0)
	    $display("PASSED");
    else
	    $display("FAILED");
  end

endmodule
