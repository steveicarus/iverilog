module top;
  reg [1:0] a, b, c, d, e;
  reg passed;

  initial begin
    a = 2'b00;
    b = 2'b00;
    c = 2'b00;
    d = 2'b00;
    e = 2'b00;
    passed =1'b1;
    #2;
    // Check that only the first process has run so far.
    if (a !== 2'b01) begin
      $display("First process in named fork has not run: %b", a);
      passed = 1'b0;
    end
    if (b !== 2'b01) begin
      $display("First process in named block has not run: %b", b);
      passed = 1'b0;
    end
    if (c !== 2'b01) begin
      $display("First process in parent ending has not run: %b", c);
      passed = 1'b0;
    end
    if (d !== 2'b01) begin
      $display("First process in parent alive has not run: %b", d);
      passed = 1'b0;
    end
    if (e !== 2'b01) begin
      $display("First process in parent ending (disable) has not run: %b", e);
      passed = 1'b0;
    end
    // This external lexical disable should disable the second process even
    // though the parent has already ended.
    disable top.be_name;
    #2;
    // Check that the second process only runs for the parent ending
    // and alive cases.
    if (a !== 2'b01) begin
      $display("Second process in named fork ran: %b", a);
      passed = 1'b0;
    end
    if (b !== 2'b01) begin
      $display("Second process in named block ran: %b", b);
      passed = 1'b0;
    end
    if (c !== 2'b11) begin
      $display("Second process in parent ending has not run: %b", c);
      passed = 1'b0;
    end
    if (d !== 2'b11) begin
      $display("Second process in parent alive has not run: %b", d);
      passed = 1'b0;
    end
    if (e !== 2'b01) begin
      $display("Second process in parent ending (disable) ran: %b", e);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end

  // Verify that disabling a named fork kills any detached processes.
  initial begin
    fork: fa_name
      #1 a[0] = 1'b1;
      #3 a[1] = 1'b1;
    join_any
    disable fa_name;
  end

  // Verify that disabling a named block kills any detached processes.
  initial begin: bb_name
    fork
      #1 b[0] = 1'b1;
      #3 b[1] = 1'b1;
    join_any
    disable bb_name;
  end

  // Verify that a detached process survives the parent ending.
  initial begin
    fork
      #1 c[0] = 1'b1;
      #3 c[1] = 1'b1;
    join_any
  end

  // Verify that a detached process runs if the parent is still alive.
  initial begin
    fork
      #1 d[0] = 1'b1;
      #3 d[1] = 1'b1;
    join_any
    #4;
  end

  // Verify that a detached process survives the parent ending, but can
  // still be disabled lexically by disabling the block that started it.
  initial begin: be_name
    fork
      #1 e[0] = 1'b1;
      #3 e[1] = 1'b1;
    join_any
  end

endmodule
