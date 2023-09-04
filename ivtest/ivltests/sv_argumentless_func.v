
module test;

function automatic [7:0] test_func;
    test_func = 8'h1;
endfunction

parameter test_parameter = test_func();

logic [7:0] test_alwayscomb;
always_comb test_alwayscomb = test_func();

logic [7:0] test_assign;
assign test_assign = test_func();

wire [7:0] test_wire = test_func();

logic [7:0] test_alwaysff;
logic clk;
always_ff @(posedge clk) test_alwaysff <= test_func();

initial begin
    if (test_func() !== 8'h1) begin
        $display("FAILED -- test_func()=%h, expect %h", test_func(), 8'h1);
        $finish;
    end

    if (test_parameter !== test_func()) begin
        $display("FAILED -- test_parameter=%h, expect %h", test_parameter, test_func());
        $finish;
    end

    #1;

    if (test_alwayscomb !== test_func()) begin
        $display("FAILED -- test_alwayscomb=%h, expect %h", test_alwayscomb, test_func());
        $finish;
    end

    if (test_assign !== test_func()) begin
        $display("FAILED -- test_assign=%h, expect %h", test_assign, test_func());
        $finish;
    end

    if (test_wire !== test_func()) begin
        $display("FAILED -- test_wire=%h, expect %h", test_wire, test_func());
        $finish;
    end

    clk = 0;
    #1;
    clk = 1;
    #1;
    if (test_alwaysff !== test_func()) begin
        $display("FAILED -- test_alwaysff=%h, expect %h", test_alwaysff, test_func());
        $finish;
    end

    $display("PASSED");
    $finish(0);
end

endmodule
