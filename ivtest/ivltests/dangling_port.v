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
// $Id: dangling_port.v,v 1.1 2001/07/08 03:22:08 sib4 Exp $
// $Log: dangling_port.v,v $
// Revision 1.1  2001/07/08 03:22:08  sib4
// Test for PR#209
//
//
// Test for PR#209, VVP wrong nodangle of dangling port.

module main;

   reg retval;
   reg a, b;

   function f;
      input dangle;
      begin
	 f = retval;
      end
   endfunction

   initial
     begin
	#1 retval <= 1;
	#1 a <= f(0);
	#1 b <= f(1);
	#1 $display("PASSED");
	$finish;
     end

endmodule
