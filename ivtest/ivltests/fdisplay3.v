/* Copyright (C) 2000 Stephen G. Tell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */
/* fdisplay3 - check that $fdisplay rejects bogus first arguments */

module fdisplay3;

   initial begin

// This error is now caught at compile time so this message will not
// be printed.
//
//      $display("expect compile or runtime error from bad $fdisplay args:");
      $fdisplay(fdisplay3, "bogus message");
      $finish;

   end // initial begin

endmodule
