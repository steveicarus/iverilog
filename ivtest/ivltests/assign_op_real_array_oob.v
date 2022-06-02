// Check that the assignment operator is supported for out-of-bounds indices on
// real arrays. The write should be skipped, but side effects of the right-hand
// side expression should still get evaluated.

module test;

  real a[1:0];
  integer i;
  real r = 0;

  function real f;
    r += 0.125;
    return r;
  endfunction

  initial begin
    a[0] = 23.0;
    a[1] = 42.0;

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
    if (a[0] == 23.0 && a[1] == 42.0 && r == 0.75) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
