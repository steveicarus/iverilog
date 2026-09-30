module test;

typedef struct packed {
    logic a;
    logic b;
} test_t;

test_t tests [0:1];

wire result_a;
wire result_b;

assign result_a = tests[0].a;
assign result_b = tests[1].b;

initial begin
    tests[0] = 2'b10;
    tests[1] = 2'b01;

    #1;

    if ((result_a === 1'b1) &&
        (result_b === 1'b1)) begin
        $display("PASSED");
    end else begin
        $display("FAILED");
        $display("result_a=%b result_b=%b",
                 result_a, result_b);
    end
end

endmodule
