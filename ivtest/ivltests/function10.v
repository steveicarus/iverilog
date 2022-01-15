/*
 * Copyright (c) 1998-2000 Andrei Purdea (andrei@purdea.ro)
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
  // Test that functions without parantheses for port-list,
  // and without any declarations compile successfully.
  // Valid according to IEEE1800-2005.
  // IEEE1364-2005 requires at least one declaration.
  function void empty_function;
  endfunction
  initial begin
    empty_function();
  end
endmodule
