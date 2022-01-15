`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

//
// Copyright (c) 1999 Steve Tell (tell@telltronics.org)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//
//  SDW - Concat in fopen not substituted correctly.

module write_sp_vectors(Clk, a, b, c);
input Clk, a, b, c;

parameter fname = "PhCount.unnamed";
parameter source_id = "(unknown source module RCSID)$";

integer fp;
initial
  begin
    // fails at runtime: "ERROR: $fopen parameter must be a constant"
    fp = $fopen({"work/",fname,".inv"});
    // this fails too
    //      fp = $fopen({"blurfl", ".inv"});
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
    $fdisplay(fp, "# captured from: %0s\n", source_id[8*80:8]);
`else
    $fdisplay(fp, "# captured from: %0s\n", source_id[$bits(source_id)-1:8]);
`endif
  end
endmodule

module main;
parameter fname = "PhCount.unnamed";

reg clk;
reg a,b,c;

write_sp_vectors #("sp2", "foo") v0 (clk,a, b, c);

initial
  begin
  #10 $finish;
  end
endmodule
