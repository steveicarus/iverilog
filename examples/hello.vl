/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
  *  Here we have the canonical "Hello, World" program written in Verilog.
  *  We simply print to the program output a simple message. This example
  *  demonstrates the process of compiling and executing a program with
  *  Icarus Verilog.
  *
  *  Compile this program with the command:
  *
  *      iverilog -ohello hello.vl
  *
  *  After churning for a little while, the program will create the output
  *  file "hello" which is compiled, linked and ready to run. Run this
  *  program like so:
  *
  *      vvp hello
  *
  *  and the program will print the message to its output. Easy! For
  *  more on how to make the iverilog command work, see the iverilog
  *  manual page.
  */

module main();

initial
  begin
    $display("Hello, World");
    $finish ;
  end

endmodule
