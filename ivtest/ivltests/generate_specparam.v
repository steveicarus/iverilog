// Check that declaring a specparam inside a generate block is an error

module test #(
  parameter A = 1
);

generate
  if (A) begin
    specparam x = 10; // Error
  end
endgenerate

initial begin
  $display("FAILED");
end

endmodule
