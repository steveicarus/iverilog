/* * Copyright (c) 1999 Daniel Nelsen (dhn@qedinc.com)
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

module main;
reg [5:0] a ;
reg error;
         // test ?: in continuous assignment with mix of constant
         // and non-constant inputs
wire [3:0] val1 = a[4] ? a[3:0] : 4'd0 ;
wire [3:0] val2 = a[4] ? 4'hf : ~a[3:0] ;

initial begin
error = 0;
 for (a = 0; a < 32; a=a+1 ) begin
     #1 ;                // wait for change in a[] to propagate to val

     if (( a[4] && (( val1 != a[3:0] ) || ( val2 != 15 ))) ||
         ( !a[4] && (( val1 != 0 ) || ( val2 != ~a[3:0] ))))
begin
             $display( "FAILED  a=%b, val1=%b, val2=%b",
                       a[4:0], val1, val2 );
error = 1;
end
end
if(error == 0)
$display("PASSED");
end

endmodule
