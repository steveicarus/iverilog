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


// Test for VHDL std.textio & ieee.std_logic_textio functions implemented using VPI.

`timescale 1ns/1ns

typedef enum integer { false, true } boolean;
typedef enum integer { read_mode , write_mode , append_mode } file_open_kind;

module vhdl_textio_test;

string line;
int file;

string str;
bit [3:0][7:0] str_lim;
real r;
int in;
integer i;
byte by;
time t;
boolean boo;

logic l;
logic [7:0] lv;
bit bi;
bit [7:0] biv;

initial begin
    static string filename = "vpi_textio_text.tmp";

    // values to be saved
    str = "test_string";
    str_lim = "TEST";
    r = -2.5e3;
    in = 120;
    i = -12;
    by = 8'h1f;
    t = 100ns;
    boo = true;
    l = 1'bx;
    lv = 8'b110101xz;
    bi = 1'b0;
    biv = 8'b10111001;

    // write test
    $ivlh_file_open(file, filename, write_mode);

    $ivlh_write(line, str, 0);     // standard format
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, str_lim, 4); // string format
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, r, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, in, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, i, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, by, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, t, 2);       // time format

    // this will be intentionally skipped during the read test
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, l, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, lv, 0);

    if(line != "test_string TEST -2500.000000 120 -12 31 100 ns X 110101XZ") begin
        $display("FAILED 1");
        $finish();
    end

    $ivlh_writeline(file, line);

    // writeline should clear the written string
    if(line != "") begin
        $display("FAILED 2");
        $finish();
    end

    $ivlh_write(line, boo, 1);     // boolean format
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, l, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, lv, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, bi, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, biv, 0);
    $ivlh_write(line, " ", 0);
    $ivlh_write(line, biv, 3);       // hex format

    if(line != "TRUE X 110101XZ 0 10111001 B9") begin
        $display("FAILED 3");
        $finish();
    end

    $ivlh_writeline(file, line);
    $fclose(file);

    // reset variables
    str = "";
    r = 0;
    in = 0;
    i = 0;
    by = 0;
    t = 0s;
    boo = false;
    l = 0;
    lv = 0;
    bi = 0;
    biv = 0;

    // read test
    $ivlh_file_open(file, filename, read_mode );

    $ivlh_readline(file, line);
    $ivlh_read(line, str, 0);     // standard format
    $ivlh_read(line, str_lim, 4); // string format
    $ivlh_read(line, r, 0);
    $ivlh_read(line, in, 0);
    $ivlh_read(line, i, 0);
    $ivlh_read(line, by, 0);
    $ivlh_read(line, t, 2);       // time format

    $ivlh_readline(file, line);
    $ivlh_read(line, boo, 1);     // boolean format
    $ivlh_read(line, l, 0);
    $ivlh_read(line, lv, 0);
    $ivlh_read(line, bi, 0);
    $ivlh_read(line, biv, 0);
    $ivlh_read(line, biv, 3);     // hex format

    $fclose(file);

    // compare read and expected values
    if(str != "test_string") begin
        $display("FAILED 5");
        $finish();
    end

    if(str_lim != "TEST") begin
        $display("FAILED 6");
        $finish();
    end

    if(r != -2.5e3) begin
        $display("FAILED 7");
        $finish();
    end

    if(in !== 120) begin
        $display("FAILED 8");
        $finish();
    end

    if(i !== -12) begin
        $display("FAILED 9");
        $finish();
    end

    if(by !== 8'h1f) begin
        $display("FAILED 10");
        $finish();
    end

    if(t != 100ns) begin
        $display("FAILED 11");
        $finish();
    end

    if(boo !== true) begin
        $display("FAILED 12");
        $finish();
    end

    if(l !== 1'bx) begin
        $display("FAILED 13");
        $finish();
    end

    if(lv !== 8'b110101xz) begin
        $display("FAILED 14");
        $finish();
    end

    if(bi !== 1'b0) begin
        $display("FAILED 15");
        $finish();
    end

    if(biv !== 8'b10111001) begin
        $display("FAILED 16");
        $finish();
    end

    $display("PASSED");
end

endmodule
