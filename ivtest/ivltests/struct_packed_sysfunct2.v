// Check that the signedness of a struct member is handled correctly when passed
// to a system function

module test;

  bit failed = 1'b0;

  struct packed {
    int s;
    int unsigned u;
  } x;

  int s;
  int unsigned u;

  logic [16*8-1:0] s1;
  logic [16*8-1:0] s2;

  initial begin
    u = -10;
    s = -20;
    x.u = u;
    x.s = s;

    $swrite(s1, s);
    $swrite(s2, x.s);

    if (s1 != s2) begin
      failed = 1'b1;
      $display("FAILED. Expected %s, got %s.", s1, s2);
    end

    $swrite(s1, u);
    $swrite(s2, x.u);

    if (s1 != s2) begin
      failed = 1'b1;
      $display("FAILED. Expected %s, got %s.", s1, s2);
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
