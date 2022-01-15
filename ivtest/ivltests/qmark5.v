/*
 * Copyright (c) 1999 Daniel Nelsen (dhn@qedinc.com)
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
 // SDW - Modified to say FAILED - or PASSED

module main;
    reg [7:0] a ;
    reg x, y ;
    reg error;
    initial
      begin
        error = 0;
	    for (a = 0; a < 32; a=a+1 )
          begin
			// ternary operator is right associative
	        x = a[0] ? a[1] : a[2] ? a[3] : a[4] ;
	        y = a[0] ? a[1] : ( a[2] ? a[3] : a[4] );
	        if ( x != y )
              begin
	            $display( "FAILED  a=%b, x=%b != y=%b", a[4:0], x, y );
                error =1;
              end
          end
        if(error == 0)
          $display("PASSED");
     end

endmodule
