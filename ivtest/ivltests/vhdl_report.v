// Copyright (c) 2015 CERN
// Maciej Suminski <maciej.suminski@cern.ch>
//
// This source code is free software; you can redistribute it
// and/or modify it in source code form under the terms of the GNU
// General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA


// Report & assert tests.

module vhdl_report_test;

logic start_test;
vhdl_report dut(start_test);
int a;

initial begin
    // as of the moment of writing vhdlpp does not handle procedure calls
    a = vhdl_report_pkg::test_asserts(0);

    start_test = 1'b0;
    #1 start_test = 1'b1;
end
endmodule
