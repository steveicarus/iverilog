// Check that end labels on unnamed blocks generate an error

module test;

  generate
    if (1) begin
    end : label
  endgenerate

  initial begin
  end : label

  initial fork
  join : label

endmodule
