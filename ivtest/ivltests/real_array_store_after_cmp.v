// Check that a store to am entry of real typed array with an immediate index
// works if it happes after a comparison that sets vvp flag 4 to 0.

module test;

  integer a = 0;
  real r[1:0];

  initial begin
    if (a == 0) begin
      // Make sure that this store happens, even though the compare above
      // cleared set vvp flag 4
      r[0] = 1.23;
    end

    if (r[0] == 1.23)  begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected %f, got %f", 1.23, r[0]);
    end
  end
endmodule

