// Check that it is possible to call functins with empty arguments if they have
// default values. Check that this works if the function call is part of a force
// statement in an automatic context.

module test;

  bit failed = 1'b0;

  `define check(expr, val) \
    if (expr !== val) begin \
      $display("FAILED. %s, expected %d, got %d", `"expr`", val, expr); \
      failed = 1'b1; \
    end

  integer x, y, z, w;

  function automatic integer f(integer a = 1, integer b = 2, integer c = 3);
    return a * 100 + b * 10 + c;
  endfunction

  task automatic t;
    force x = f(4,  , 6);
    force y = f(4, 5,  );
    force z = f(4,  ,  );
    force w = f( ,  , 6);
  endtask

  initial begin
    t;

    `check(x, 426);
    `check(y, 453);
    `check(z, 423);
    `check(w, 126);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
