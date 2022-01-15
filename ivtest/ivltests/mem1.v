/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 *
 * $Log: mem1.v,v $
 * Revision 1.2  2001/01/29 17:26:06  ka6s
 * Check in fixes contributed by Paul Campbell (Thanks Paul)
 *
 */

`define CLK 10

module main;
    reg     [31:0]      counter;
    integer             i;
    reg     [23:0]      testvec [15:0];
    reg                 clk;
    wire    [23:0]      data;
    reg                 write;


    initial
    begin
        write = 0;
        counter = 0;
        clk = 0;

        $readmemb("ivltests/mem1.dat", testvec, 0);

        for (i = 0; i < 16; i = i + 1)
        begin
            $write("mem[%d] = %x\n", i, testvec[i]);
        end
    end

    always
    begin
        #`CLK clk = ~clk;
    end

    assign data = (write) ? testvec[counter] : 24'bz;

    always @ (posedge clk)
    begin
        begin
            write = 1;
            #1 ;
            $write("%d %x\n", counter, data);
            write = 0;
            counter = counter + 1;
            if (counter == 16)
                $finish(0);
        end
    end

endmodule
