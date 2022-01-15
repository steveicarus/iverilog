/*
 * Copyright (c) 2000 Steve Wilson (stevew@home.com)
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
 * Verify signed constant assignment to an integer.
 */
module test;

integer I0,I1;
reg [15:0] R0,R1;
reg [3:0] error;

initial
  begin
    error = 0;
    I0 = -4'd12;
    I1 = -32'd12;
    if(I0 !== 32'hfffffff4)
      begin
        $display("FAILED - negative decimal assignment failed. I0 s/b fffffff4, is %h",
                I0);
        error =1;
      end
    if(I1 !== 32'hfffffff4)
      begin
         $display("FAILED - negative decimal assignment failed. I1 s/b fffffff4, is %h",
                I1);
        error = 1;
      end
    if(error === 0)
      $display("PASSED");
  end

endmodule
