// Check that a range applies to every specparam in a declaration.

module test;

  specparam [3:0] A = 5'b10101, B = 5'b11010;
  specparam C = 5'b10011;

  initial begin
    if (A !== 4'd5 || B !== 4'd10 || C !== 5'd19) begin
      $display("FAILED: A=%0d B=%0d C=%0d", A, B, C);
    end else begin
      $display("PASSED");
    end
  end

endmodule
