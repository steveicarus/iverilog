module dut;
  reg res;

  function f(input a);
    f = 1'b0;
  endfunction

  initial begin
    res = ((2'd1 ? f(1'b0) : 1'b0) ? 1'b1 : 1'b0);
    if (res !== 1'b0) $display("FAILED: Expected 1'b0, got %b", res);
    else $display("PASSED");
  end
endmodule
