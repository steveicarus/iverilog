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

    reg [5:0]	addr;
    reg [31:0]	m_poke[4:0];
    reg [31:0]	m_peek[4:0];

    task f_init;
	integer i;
    begin
	for (i = 0; i < 5; i = i + 1) begin
	    m_poke[i] = $random;
	end
    end
    endtask

    task f_copy;
	integer i;
    begin
	for (i = 0; i < 5; i = i + 1) begin
	    m_peek[i] = m_poke[i];
	end
    end
    endtask

    task f_dump;
	integer i;
    begin
	for (i = 0; i < 5; i = i + 1) begin
	    $display ("m_poke[%0d] <=> m_peek[%0d] 0x%x 0x%x%s",
		i, i, m_poke[i], m_peek[i],
		m_poke[i] !== ~m_peek[i] ? " - ERROR" : "");
	end
    end
    endtask

    initial begin
	// f_init;
	#0;
	$mempoke;
	#10;
	f_copy;
	#10;
	$mempeek;
	#10;
	f_dump;
    end

endmodule
