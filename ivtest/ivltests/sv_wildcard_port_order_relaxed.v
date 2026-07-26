// Check relaxed declaration ordering for wildcard port connections.
// This is not valid in strict Verilog and tests
// -gno-strict-net-var-declaration.

module child(input wire source = 1'b0, input wire value = 1'b0,
             output wire result);

  assign result = value;

endmodule

module test;

  wire result;
  child i_child(.result(result), .*, .source(value));

  assign value = 1'b1;

  initial begin
    #1;

    if (result !== 1'b1) begin
      $display("FAILED: expected 1, got %b", result);
    end else begin
      $display("PASSED");
    end
  end

endmodule
