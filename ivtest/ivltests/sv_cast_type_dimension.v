// Check an inline packed dimension in a type cast.

module test;

  logic [7:0] result;

  initial begin
    result = logic [3:0]'(8'haf);

    if (result !== 8'h0f) begin
      $display("FAILED(%0d). Expected 0f, got %h", `__LINE__, result);
    end else begin
      $display("PASSED");
    end
  end

endmodule
