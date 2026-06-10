module top;
    logic [9:0] pipe;
    logic [4:0] i;
    logic clk = 0;
    logic rst = 1;

    always #1 clk = ~clk;

    always_ff @(posedge clk) begin
        if (rst) begin
            pipe <= 10'b0000000001;
        end else begin
            for (i=0; i<9; i++) begin
                pipe[i+1] <= pipe[i];
            end
            pipe[0] <= pipe[9];
        end
    end

    initial begin
        #2 rst = 0;

        for (int j=0; j<10; j++) begin
            $display(pipe[9]);
            #2;
        end

        $finish(0);
    end
endmodule
