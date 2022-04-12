module top;

    logic clk = 0;
    int cnt = 0;

    always @(posedge clk) begin
        fork begin
            #(10*2); // Wait 10 clock periods
            cnt++;
        end
        join_none
    end

    initial begin
        $display("Starting test");
        repeat (100) begin
            #1 clk = 1;
            #1 clk = 0;
        end
        #(10*2); // Wait 10 clock periods
        $display("cnt = %0d", cnt);
        if (cnt === 100)
            $display("PASSED");
        else
            $display("FAILED");
        $finish(0);
    end
endmodule
