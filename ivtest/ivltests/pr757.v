module  main;

    reg     [11:0]  sum;
    wire    [10:0]  a = 11'b111_0000_0000;
    wire    [10:0]  b = 11'b000_0000_1111;

    initial begin
        #1 sum  = $signed(a) + $signed(b);

        if (sum == 12'b1111_0000_1111)
            $display("PASSED");
        else
            $display("failed: %b",sum);

        $finish;
    end

endmodule
