// Copyright (c) 2026 Jerry Zhong
// SPDX-License-Identifier: GPL-2.0-or-later
`timescale 1ns/1ps
module cleanup_leaf(a, y);
    input a;
    output y;
    parameter integer BIAS = 7;
    reg [7:0] memory [0:1];
    integer total;
    integer event_count;
    real ratio;
    event tick;
    assign y = ~a;

    function automatic integer plus_two;
        input integer value;
        plus_two = value + 2;
    endfunction

    task automatic record;
        input integer value;
        begin
            #1 total = plus_two(value);
            -> tick;
        end
    endtask

    generate if (1) begin : generated
        reg marker;
        initial marker = 1;
    end endgenerate

    always @(tick) event_count = event_count + 1;
    initial begin : active
        total = 0;
        event_count = 0;
        memory[0] = 8'h03;
        memory[1] = 8'h09;
        ratio = 1.5;
        record(10);
    end

    initial begin : fork_owner
        fork : sleepers
            begin : left
                #100 total = 100;
            end
            begin : right
                #200 total = 200;
            end
        join
    end
endmodule

module tb;
    reg a;
    wire y;
    cleanup_leaf u(a, y);
    initial begin
        a = 0;
        #2;
        if (y !== 1'b1 || u.total !== 12 || u.event_count !== 1 ||
            u.memory[0] !== 8'h03 || u.memory[1] !== 8'h09 ||
            u.ratio != 1.5 || u.BIAS != 7 || u.generated.marker !== 1'b1)
            $fatal(1, "FAILED cleanup functional initialization");
        a = 1;
        #2;
        if (y !== 1'b0 || u.total !== 12 || u.event_count !== 1)
            $fatal(1, "FAILED cleanup functional propagation");
        if ($cleanup_inventory() !== 1)
            $fatal(1, "FAILED cleanup inventory");
        $display("PASSED");
        $finish;
    end
endmodule
