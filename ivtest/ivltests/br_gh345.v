// Based on https://github.com/YosysHQ/yosys/blob/master/tests/various/const_func.v
//
// ISC License
//
// Copyright (C) 2012 - 2020  Claire Wolf <claire@symbioticeda.com>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

module Example(outA, outB, outC, outD);
    parameter OUTPUT = "FOO";
    output wire [23:0] outA;
    output wire [23:0] outB;
    output reg outC, outD;
    function automatic [23:0] flip;
        input [23:0] inp;
        flip = ~inp;
    endfunction

    generate
        if (flip(OUTPUT) == flip("BAR"))
            assign outA = OUTPUT;
        else
            assign outA = 0;

        case (flip(OUTPUT))
            flip("FOO"): assign outB = OUTPUT;
            flip("BAR"): assign outB = 0;
            flip("BAZ"): assign outB = "HI";
        endcase

        genvar i;
        initial outC = 0;
        for (i = 0; i != flip(flip(OUTPUT[15:8])); i = i + 1)
            if (i + 1 == flip(flip("O")))
                initial #1 outC = 1;
    endgenerate

    integer j;
    initial begin
        outD = 1;
        for (j = 0; j != flip(flip(OUTPUT[15:8])); j = j + 1)
            if (j + 1 == flip(flip("O")))
                outD = 0;
    end
endmodule

module top(out);
    wire [23:0] a1, a2, a3, a4;
    wire [23:0] b1, b2, b3, b4;
    wire        c1, c2, c3, c4;
    wire        d1, d2, d3, d4;
    Example          e1(a1, b1, c1, d1);
    Example #("FOO") e2(a2, b2, c2, d2);
    Example #("BAR") e3(a3, b3, c3, d3);
    Example #("BAZ") e4(a4, b4, c4, d4);

    output wire [24 * 8 - 1 + 4 :0] out;
    assign out = {
        a1, a2, a3, a4,
        b1, b2, b3, b4,
        c1, c2, c3, c4,
        d1, d2, d3, d4};

    initial begin
        #2;
        $display("%h %h %h %h", a1, a2, a3, a4);
        $display("%h %h %h %h", b1, b2, b3, b4);
        $display(c1,,c2,,c3,,c4);
        $display(d1,,d2,,d3,,d4);
        if((a1 === 0)
        && (a2 === 0)
        && (a3 === "BAR")
        && (a4 === 0)
        && (b1 === "FOO")
        && (b2 === "FOO")
        && (b3 === 0)
        && (b4 === "HI")
        && (c1 === 1)
        && (c2 === 1)
        && (c3 === 0)
        && (c4 === 0)
        && (d1 === 0)
        && (d2 === 0)
        && (d3 === 1)
        && (d4 === 1))
            $display("PASSED");
        else
            $display("FAILED");
    end
endmodule
