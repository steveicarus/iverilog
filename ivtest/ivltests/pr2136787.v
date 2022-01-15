module signed_mux_bug();

reg       s;
reg [3:0] a, b;
reg [7:0] y, z;

initial begin
    // Example vector
    s = 1'b1;
    a = 4'b1010;
    b = 4'b0000;

    // Manually sign extend operands before multiplexer
    y = s ? {{4{a[3]}}, a} : {{4{b[3]}}, b};

    // Use $signed() to sign extend operands before multiplexer
    //   - Note that Icarus is not sign extending as expected
    z = s ? $signed(a) : $signed(b);

    // Display results
    $display("a = %b", a);
    $display("b = %b", b);
    $display("y = %b", y);
    $display("z = %b", z);

    // Finished
    $finish(0);
end

endmodule
