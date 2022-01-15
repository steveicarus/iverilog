// Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
//		    Michael Runyan (mrunyan at chiaro.com)
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

    reg [7:0]	r_poke_1, r_poke_2, r_poke_3, r_poke_4, r_poke_5;
    reg [7:0]	r_peek_1, r_peek_2, r_peek_3, r_peek_4, r_peek_5;

    task f_copy;
    begin
	// twizzle copy
	r_peek_1 = r_poke_2;
	r_peek_2 = r_poke_3;
	r_peek_3 = r_poke_4;
	r_peek_4 = r_poke_5;
	r_peek_5 = r_poke_1;
    end
    endtask

    task f_dump;
	integer i;
    begin
	$display("Verilog compare r_poke <=> r_peek");
	$display ("  'b_%b <=> 'b_%b%s",
	    r_poke_1, r_peek_5, r_poke_1 !== r_peek_5 ? " - ERROR" : "");
	$display ("  'b_%b <=> 'b_%b%s",
	    r_poke_2, r_peek_1, r_poke_2 !== r_peek_1 ? " - ERROR" : "");
	$display ("  'b_%b <=> 'b_%b%s",
	    r_poke_3, r_peek_2, r_poke_3 !== r_peek_2 ? " - ERROR" : "");
	$display ("  'b_%b <=> 'b_%b%s",
	    r_poke_4, r_peek_3, r_poke_4 !== r_peek_3 ? " - ERROR" : "");
	$display ("  'b_%b <=> 'b_%b%s",
	    r_poke_5, r_peek_4, r_poke_5 !== r_peek_4 ? " - ERROR" : "");
    end
    endtask

    initial begin
	#0;
	$regpoke;
	#10;
	f_copy;
	#10;
	$regpeek;
	#10;
	f_dump;
    end

endmodule
