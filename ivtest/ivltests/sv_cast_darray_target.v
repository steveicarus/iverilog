// Check that typed elaboration uses the explicit cast target type.

module test;

  typedef logic scalar_t;
  typedef scalar_t [7:0] packed_byte_t;
  typedef logic [7:0] byte_array_t[];

  packed_byte_t result[];
  bit failed = 1'b0;

  initial begin
    result = byte_array_t'(16'h12ab);

    if (result.size() != 2 || result[0] !== 8'h12 ||
	result[1] !== 8'hab) begin
      $display("FAILED(%0d). Incorrect result", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
