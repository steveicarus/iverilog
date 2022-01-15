/*
 * Copyright (c) 2001 Brendan J Simon
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

//****************************************************************************
//
//  MODULE      :   parameter_test
//
//  DESCRIPTION :   Test module to demonstrate parameter evaluation bug.
//
//  AUTHOR      :   Brendan J Simon (brendan.simon@bigpond.com)
//
//  DATE        :   Monday 5th January 2001.
//
//  NOTES       :   It seems that Icarus Verilog 0.4 does not evaluate
//                  moderately complex parameter statements properly.
//
//****************************************************************************

module parameter_test;


parameter   foo_size    = 32 * 6;
parameter   foo_lsb     = 0;

`ifdef GOOD_CODE
    parameter   foo_msb_temp    = foo_lsb + foo_size;
    parameter   foo_msb         = foo_msb_temp - 1;
`else
    parameter   foo_msb         = foo_lsb + foo_size - 1;
`endif


// These complex statements work;
parameter temp0 = 1 + 2 + 3 + 4 + 5;
parameter temp1 = 1 + 2 + 3 + 4 + 5 - 1;

reg [foo_msb:foo_lsb] foo;
integer i;


initial begin
    for (i=0; i<foo_size; i=i+1) begin
        foo[i] = 1;
    end
    $write("foo = %0h\n", foo);
    $finish(0);
end


endmodule

//****************************************************************************
//  EOF : parameter_test
//****************************************************************************
