
module test;

function automatic [7:0] test_func;
    input _unused;
    test_func = _unused;
endfunction

logic [7:0] test_assign;
assign test_assign = test_func(0);

wire [7:0] test_wire = test_func(0);

initial begin
    if (test_assign !== test_func(0)) begin
        $display("FAILED -- test_assign=%h, expect %h", test_assign, test_func(0));
        $finish;
    end

    if (test_wire !== test_func(0)) begin
        $display("FAILED -- test_wire=%h, expect %h", test_wire, test_func(0));
        $finish;
    end

    $display("PASSED");
    $finish;
end

endmodule
