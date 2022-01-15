// Copyright (c) 2014 CERN
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


// Test for typedefs using unpacked arrays (including dynamic arrays).

module unp_array_typedef();
    typedef logic [7:0] bit_darray [];
    typedef logic [15:0] bit_unparray [4];
    typedef string string_darray [];
    typedef string string_unparray [2];
    typedef real real_darray [];
    typedef real real_unparray [5];
    typedef int int_darray [];
    typedef int int_unparray [3];
    typedef struct packed {
       logic [7:0] high;
       logic [7:0] low;
    } word;
    typedef word word_darray [];
    typedef word word_unparray [2];

    bit_darray bit_darr;
    bit_unparray bit_unparr;
    string_darray string_darr;
    string_unparray string_unparr;
    real_darray real_darr;
    real_unparray real_unparr;
    int_darray int_darr;
    int_unparray int_unparr;
    word_darray word_darr;
    word_unparray word_unparr;

initial begin
    // Bit type
    bit_darr = new[3];
    bit_darr[0] = "a";
    bit_darr[1] = "b";
    bit_darr[2] = "c";

    if(bit_darr[0] !== "a" || bit_darr[1] !== "b" || bit_darr[2] !== "c")
    begin
        $display("FAILED 1");
        $finish();
    end

    bit_unparr[0] = 16'h1234;
    bit_unparr[1] = 16'h5678;
    bit_unparr[2] = 16'h9abc;

    if(bit_unparr[0] !== 16'h1234 || bit_unparr[1] !== 16'h5678 || bit_unparr[2] !== 16'h9abc)
    begin
        $display("FAILED 2");
        $finish();
    end

    // String type
    string_darr = new[3];
    string_darr[0] = "icarus";
    string_darr[1] = "verilog";
    string_darr[2] = "test";

    if(string_darr[0] != "icarus" || string_darr[1] != "verilog" || string_darr[2] != "test") begin
        $display("FAILED 3");
        $finish();
    end

    string_unparr[0] = "test_string";
    string_unparr[1] = "another test";

    if(string_unparr[0] != "test_string" || string_unparr[1] != "another test") begin
        $display("FAILED 4");
        $finish();
    end

    // Real type
    real_darr = new[3];
    real_darr[0] = -1.20;
    real_darr[1] = 2.43;
    real_darr[2] = 7.4;

    if(real_darr[0] != -1.20 || real_darr[1] != 2.43 || real_darr[2] != 7.4) begin
        $display("FAILED 5");
        $finish();
    end

    real_unparr[0] = 1.0;
    real_unparr[1] = 2.5;
    real_unparr[2] = 3.0;
    real_unparr[3] = 4.5;
    real_unparr[4] = 5.0;

    if(real_unparr[0] != 1.0 || real_unparr[1] != 2.5 || real_unparr[2] != 3.0 ||
            real_unparr[3] != 4.5 || real_unparr[4] != 5.0) begin
        $display("FAILED 6");
        $finish();
    end

    // Integer type
    int_darr = new[3];
    int_darr[0] = -3;
    int_darr[1] = 3;
    int_darr[2] = 72;

    if(int_darr[0] !== -3 || int_darr[1] != 3 || int_darr[2] != 72) begin
        $display("FAILED 7");
        $finish();
    end

    int_unparr[0] = 22;
    int_unparr[1] = 18;
    int_unparr[2] = 9;

    if(int_unparr[0] !== 22 || int_unparr[1] != 18 || int_unparr[2] != 9) begin
        $display("FAILED 8");
        $finish();
    end

    // Struct
    // TODO at the moment dynamic arrays of struct are not supported
    /* word_darr = new[3];
    word_darr[0].high = 8'h11;
    word_darr[0].low  = 8'h22;
    word_darr[1].high = 8'h33;
    word_darr[1].low  = 8'h44;
    word_darr[2].high = 8'h55;
    word_darr[2].low  = 8'h66;

    if(word_darr[0].low !== 8'h22 || word_darr[0].high !== 8'h11 ||
            word_darr[1].low !== 8'h44 || word_darr[1].high !== 8'h33) begin
            word_darr[2].low !== 8'h66 || word_darr[2].high !== 8'h55) begin
        $display("FAILED 9");
        $finish();
    end*/

    // TODO not available at the moment
    //word_unparr[0].high = 8'haa;
    //word_unparr[0].low  = 8'h55;
    //word_unparr[1].high = 8'h02;
    //word_unparr[1].low  = 8'h01;

    //if(word_unparr[0].low !== 8'h55 || word_unparr[0].high !== 8'haa ||
            //word_unparr[1].low !== 8'h01 || word_unparr[1].high !== 8'h02) begin
        //$display("FAILED 10");
        //$finish();
    //end

    word_unparr[0] = 16'haa55;
    word_unparr[1] = 16'h0102;

    if(word_unparr[0] !== 16'haa55 || word_unparr[1] !== 16'h0102)
    begin
        $display("FAILED 10");
        $finish();
    end

    $display("PASSED");
end
endmodule

