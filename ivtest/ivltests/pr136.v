//****************************************************************************
//
//  MODULE      :   parameter_multiply_test
//
//  DESCRIPTION :   Test module to demonstrate parameter multiplication bug.
//
//  AUTHOR      :   Brendan J Simon (brendan.simon@bigpond.com)
//
//  DATE        :   Tuesday 6th January 2001.
//
//  NOTES       :   It seems that Icarus Verilog 0.4 does not evaluate
//                  parameter multiplication properly.
//                  The code compiles OK, but gives a runtime error of:
//                  vpi_const.c:35: vpip_bits_to_dec_str: Assertion `nbits <=
//                  8*sizeof(val)' failed.
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
//   SDW: multiply in parameter
//****************************************************************************

module parameter_multiply_test;

parameter   foo_size    = 4 * 8;

reg [31:0] testv;
initial begin
    testv = foo_size;
    if(testv !== 32)
      begin
            $write("foo_size = %d\n", testv);
	    $display("FAILED");
      end
    else
      begin
            $write("foo_size = %d\n", testv);
	    $display("PASSED");
      end
    $finish;
end


endmodule

//****************************************************************************
//  EOF : parameter_multiply_test
//****************************************************************************
