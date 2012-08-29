/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * IVL should generate an AND gate, and should make an OBUF and two
 * IBUF objects, along with the PAD objects.
 *
 * To compile this for XNF, try a command like this:
 *
 *    iverilog -txnf -ppart=XC4010XLPQ160 -ooutff.xnf -pncf=outff.ncf outff.v
 *
 * That command causes an outff.xnf and outff.ncf file to be created.
 * Next, make the outff.ngd file with the command:
 *
 *    xnf2ngd -l xilinxun -u outff.xnf outff.ngo
 *    ngdbuild outff.ngo outff.ngd
 *
 * Finally, map the file to fully render it in the target part. The
 * par command is the step that actually optimizes the design and tries
 * to meet timing constraints.
 *
 *    map -o map.ncd outff.ngd
 *    par -w map.ncd outff.ncd
 *
 * At this point, you can use the FPGA Editor to edit the outff.ncd
 * file to see that the AND gate is in a CLB and the IOB for pin 150
 * has its flip-flop in use, and that gbuf is a global buffer.
 */

module main;

   wire clk, iclk;
   wire i0, i1;
   wire out;
   reg o0;

      // This simple logic gate get turned into a function unit.
      // The par program will map this into a CLB F or G unit.
   and (out, i0, i1);

     // This creates a global clock buffer. Notice how I attach an
     // attribute to the named gate to force it to be mapped to the
     // desired XNF device. This device will not be pulled into the
     // IOB associated with iclk because of the attribute.
   buf gbuf(clk, iclk);
   $attribute(gbuf, "XNF-LCA", "GCLK:O,I");

      // This is mapped to a DFF. Since o0 is connected to a PAD, it
      // is turned into a OUTFF so that it get placed into an IOB.
   always @(posedge clk) o0 = out;

      // These attribute commands assign pins to the listed wires.
      // This can be done to wires and registers, as internally both
      // are treated as named signals.
   $attribute(o0,  "PAD", "o150");
   $attribute(i0,  "PAD", "i152");
   $attribute(i1,  "PAD", "i153");
   $attribute(iclk,"PAD", "i154");

endmodule /* main */
