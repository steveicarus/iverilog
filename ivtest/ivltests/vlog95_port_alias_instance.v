// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
// A caller's tied inputs do not make two formal ports intrinsic aliases.
`timescale 1ps/1ps
module plain_two_inputs(A, B, Y, Z);
    input A, B;
    output Y, Z;
    assign Y = A;
    assign Z = B;
endmodule
module tb;
    reg tied, C, D;
    wire TY, TZ, Y, Z;
    plain_two_inputs a_tied(tied, tied, TY, TZ);
    plain_two_inputs z_distinct(C, D, Y, Z);
    task check;
        input et, ec, ed;
        begin
            tied = et; C = ec; D = ed; #1;
            if ({TY, TZ, a_tied.A, a_tied.B} !== {et, et, et, et})
                $fatal(1, "FAILED tied-input instance");
            if ({Y, Z, z_distinct.A, z_distinct.B} !== {ec, ed, ec, ed})
                $fatal(1, "FAILED distinct-input instance");
        end
    endtask
    initial begin
        check(0, 0, 1);
        check(1, 1, 0);
        check(1'bx, 0, 1);
        check(1'bz, 1, 0);
        check(0, 1'bx, 1'bz);
        check(1, 1'bz, 1'bx);
        $display("PASSED");
        $finish;
    end
endmodule
