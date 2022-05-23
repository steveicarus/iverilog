// Check that the assignment operator is supported for out-of-bounds indices.
// The write should be skipped, but side effects of the right-hand side
// expression should still get evaluated.

module test;

  // Check that wider than 32 works
  logic [39:0] a[1:0];
  integer i;
  logic [39:0] j = 0;

  function logic [39:0] f;
    j++;
    return j;
  endfunction

  initial begin
    a[0] = 23;
    a[1] = 42;

    // Immediate out-of-bounds indices
    a[-1] += f();
    a[2] += f();
    a['hx] += f();

    // Variable out-of-bounds indices
    i = -1;
    a[i] += f();
    i = 2;
    a[i] += f();
    i = 'hx;
    a[i] += f();

    // Check that the in-bounds elements do not get affected by out-of-bounds
    // updates. Check that the left-hand side of the operator assignment gets
    // evaluated.
    if (a[0] == 23 && a[1] == 42 && j == 6) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
