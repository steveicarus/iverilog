module top;
  // This should be valid. The Icarus compiler keeps these as negatives, but
  // the run time doesn't support negative values.
  enum bit signed [7:0] {rn = -1, yn = -2, gn = -3} nl;
  integer val;

  initial begin
    nl = rn;
    $display("First:   %d", nl);
    nl = nl.next;
    $display("Second:  %d", nl);
    nl = nl.next;
    $display("Third:   %d", nl);
    nl = nl.next;
    $display("Wrapped: %d", nl);
    nl = nl.prev;
    $display("Wrapped: %d", nl);
    val = nl;
    $display("As integer: %d", val);
  end

  // This should be a signed value!
  initial #1 $display("Compile: ", rn);

endmodule
