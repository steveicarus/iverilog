module tranif_or(a, b, y);
    input a, b;
    inout y;

    supply1 vdd;
    supply0 vss;

    wire w1, w2;

    // NOR
    tranif0(w1, vdd, a);
    tranif0(w2, w1,  b);
    tranif1(w2, vss, a);
    tranif1(w2, vss, b);

    // OR
    tranif0(y, vdd, w2);
    tranif1(y, vss, w2);
endmodule

module test;

reg     a, b;
wire    y;

tranif_or dut(a, b, y);

reg failed = 0;

initial begin
    $monitor("%t a=%b b=%b Y=%b", $time, a, b, y);
    #10 a = 0; b = 0;
    #0 if (y !== 0) failed = 1;
    #10 a = 0; b = 1;
    #0 if (y !== 1) failed = 1;
    #10 a = 1; b = 0;
    #0 if (y !== 1) failed = 1;
    #10 a = 1; b = 1;
    #0 if (y !== 1) failed = 1;
    #1;
    if (failed)
        $display("FAILED");
    else
        $display("PASSED");
end

endmodule


