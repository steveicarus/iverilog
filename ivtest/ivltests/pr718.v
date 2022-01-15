//
// Verifies disable terminates a forked forever.
//
module test;
    initial begin
        fork: F
            forever #10;
            disable F;
        join
        $display("PASSED");
        $finish;
    end

    initial begin
        #20;
        $display("FAILED");
        $finish;
    end

endmodule
