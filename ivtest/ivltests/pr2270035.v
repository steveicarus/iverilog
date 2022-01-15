module test ();
    reg [30:0] a, b;

    initial begin
        b = 1;
        a = (0 << b);
        // $display ("a: %d", a);
        if (a !== 31'b0) $display("FAILED");
        else $display("PASSED");
    end
endmodule
