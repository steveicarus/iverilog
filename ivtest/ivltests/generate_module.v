// Check that declaring a module inside a generate block is an error

module test #(
  parameter A = 1
);

generate
  if (A) begin
    // Error
    module inner;
      initial $display("FAILED");
    endmodule
  end
endgenerate

endmodule
