// Check that a case statement cannot have multiple default items.

module test;
  reg value;

  initial begin
    case (0) // Error: Multiple default items in one case statement.
      default: value = 1'b0;
      1: value = 1'b1;
      default: value = 1'b1;
    endcase
    $display("FAILED");
  end
endmodule
