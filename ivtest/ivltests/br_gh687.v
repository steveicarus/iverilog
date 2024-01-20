module top;
    logic [9:0] pipe = 0;
    logic [4:0] i;
    logic clk = 0;

    always #1 clk = ~clk;

    always_ff @(posedge clk) begin
        for (i=0; i<9; i++) begin
            pipe[i+1] <= pipe[i];
        end
        pipe[0] <= pipe[9];
    end

    initial begin
        pipe[0] = 1'b1;

        for (int j=0; j<10; j++) begin
            $display(pipe[9]);
            #2;
        end

        $finish(0);
    end
endmodule
