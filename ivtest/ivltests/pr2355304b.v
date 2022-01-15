module test2 ();
    reg [1:0] d;

    always @(posedge |d) begin
        $display ("PASSED");
    end
    initial begin
        d=0;
        # 1;
        d=6'b01;
    end
endmodule
