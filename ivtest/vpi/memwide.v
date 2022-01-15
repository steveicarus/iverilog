// Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
//                    Michael Runyan (mrunyan at chiaro.com)
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

module test;

  reg [0:32]	big_reg;
  reg [0:32]	my_mem[0:16];
  reg		event_trigger;

  initial begin
    #10 $display("!!!VERILOG: big_reg=%h",big_reg);
        $display("            my_mem[1]=%h",my_mem[1]);
        event_trigger=1;
    #10 big_reg=33'h1_2345_6789;
        my_mem[1]=33'h1_5432_9876;
    #10 $display("!!!VERILOG: big_reg=%h",big_reg);
        $display("            my_mem[1]=%h",my_mem[1]);
        event_trigger=!event_trigger;
    #10 $finish(0);
  end

endmodule
