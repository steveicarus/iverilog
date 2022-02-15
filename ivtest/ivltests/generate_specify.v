// Check that a specify block inside a generate block is an error

module test #(
  parameter A = 1
);

generate
  if (A) begin
    specify // Error
    endspecify
  end
endgenerate

initial begin
  $display("FAILED");
end

endmodule
