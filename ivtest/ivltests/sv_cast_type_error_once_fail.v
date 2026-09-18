// Check that a cast target type is elaborated only once.

module test;

  logic [7:0] value;

  initial begin
    value = logic [missing:0]'(1);
  end

endmodule
