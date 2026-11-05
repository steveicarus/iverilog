// Check that %m includes named blocks inside tasks and functions.

module test;

  reg failed;
  reg [511:0] scope_name;
  integer value;

  `define check(exp) \
    $sformat(scope_name, "%m"); \
    if (scope_name !== exp) begin \
      $display("FAILED(%0d): '%%m' expected %s, got %0s", \
               `__LINE__, exp, scope_name); \
      failed = 1'b1; \
    end

  task t;
    begin
      `check("test.t");
      begin : body
        `check("test.t.body");
        begin : inner
          `check("test.t.body.inner");
        end
        `check("test.t.body");
      end
      fork : parallel
        begin
          `check("test.t.parallel");
        end
      join
      `check("test.t");
    end
  endtask

  function integer f;
    input integer arg;
    begin
      `check("test.f");
      begin : body
        `check("test.f.body");
        begin : inner
          `check("test.f.body.inner");
        end
        `check("test.f.body");
      end
      `check("test.f");
      f = arg;
    end
  endfunction

  initial begin
    failed = 1'b0;
    value = 1;
    t;
    value = f(value);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
