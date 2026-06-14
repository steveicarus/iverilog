// Check that enum literals declared inside generate blocks are available in
// the generated scope.

module test;

  reg failed;

  `define check(val, exp) do begin \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  end while (0)

  if (1) begin : g
    typedef enum logic [3:0] {
      A = 4'd1,
      B = 4'd2
    } EN_T;

    EN_T e = A;

    initial begin
      failed = 1'b0;

      #1;
      `check(e, A);

      e = B;
      `check(e, B);

      if (!failed) begin
        $display("PASSED");
      end
    end
  end

endmodule
