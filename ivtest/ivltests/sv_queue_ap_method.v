// Check that assignment patterns are supported for queue method arguments.

module test;

  bit failed;
  bit [1:0][3:0] q[$];

  `define check(val, exp) do begin \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end \
  end while (0)

  initial begin
    failed = 1'b0;

    q.push_back('{4'h1, 4'h2});
    q.push_front('{4'h3, 4'h4});
    q.insert(1, '{4'h5, 4'h6});

    `check(q.size(), 3);
    `check(q[0], 8'h34);
    `check(q[1], 8'h56);
    `check(q[2], 8'h12);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
