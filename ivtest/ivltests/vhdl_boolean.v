// Copyright (c) 2015 CERN
// @author Maciej Suminski <maciej.suminski@cern.ch>
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


// boolean values test.

module vhdl_boolean_test;

vhdl_boolean dut();

initial begin
    if(dut.true_val != \true ) begin
        $display("FAILED true 1");
        $finish();
    end

    if(!dut.true_val) begin
        $display("FAILED true 2");
        $finish();
    end


    if(dut.false_val != \false ) begin
        $display("FAILED false 1");
        $finish();
    end

    if(dut.false_val) begin
        $display("FAILED false 2");
        $finish();
    end


    if(!dut.and1) begin
        $display("FAILED and1");
        $finish();
    end

    if(dut.and2) begin
        $display("FAILED and2");
        $finish();
    end

    if(dut.and3) begin
        $display("FAILED and3");
        $finish();
    end


    if(!dut.or1) begin
        $display("FAILED or1");
        $finish();
    end

    if(!dut.or2) begin
        $display("FAILED or2");
        $finish();
    end

    if(dut.or3) begin
        $display("FAILED or3");
        $finish();
    end


    if(!dut.not1) begin
        $display("FAILED not1");
        $finish();
    end

    if(dut.not2) begin
        $display("FAILED not2");
        $finish();
    end

    $display("PASSED");
end
endmodule
