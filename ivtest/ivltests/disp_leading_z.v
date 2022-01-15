/*
 * Copyright (c) 1998 Purdea Andrei (purdeaandrei@yahoo.com)
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
 * This demonstrates proper handling of leading zeroes, and the %0b format.
 */
module main();

initial
  begin
    $display("|%b|",     10'b11);
    $display("|%0b|",    10'b11);
    $display("|%b|",     10'b0);
    $display("|%0b|",    10'b0);
    $finish(0);
  end

endmodule
