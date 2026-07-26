// Check that wildcard port connections ignore later declarations.

module child(input wire value = 1'b0, output wire result);

  assign result = value;

endmodule

module test;

  wire result;
  child i_child(.result(result), .*);
  wire value = 1'b1;

  initial begin
    #1;

    if (result !== 1'b0) begin
      $display("FAILED: expected 0, got %b", result);
    end else begin
      $display("PASSED");
    end
  end

endmodule
