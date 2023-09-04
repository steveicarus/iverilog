module top;
  reg passed;
  wire wconst, wfconst, wfconstarg;
  wire wdconst = 1'b0;
  wire wdfconst = cfunc();
  wire wdfconstarg = func(1'b0);
  real rfconst;

  function automatic reg cfunc();
    cfunc = 1'b0;
  endfunction

  function automatic reg func(input reg val);
    func = val;
  endfunction

  function automatic real crfunc();
    crfunc = 2.0;
  endfunction

  assign wconst = 1'b1;
  assign wfconst = cfunc();
  assign wfconstarg = func(1'b1);
  assign rfconst = crfunc();

  initial begin
    passed = 1'b1;
    #1;

    if (wconst !== 1'b1) begin
      $display("Expected wire constant value to be 1'b1, actual is %b", wconst);
      passed = 1'b0;
    end

    if (wdconst !== 1'b0) begin
      $display("Expected wire decl constant value to be 1'b0, actual is %b", wdconst);
      passed = 1'b0;
    end

    if (wfconst !== 1'b0) begin
      $display("Expected wire constant function value to be 1'b0, actual is %b", wfconst);
      passed = 1'b0;
    end

    if (wdfconst !== 1'b0) begin
      $display("Expected wire decl constant function value to be 1'b0, actual is %b", wdfconst);
      passed = 1'b0;
    end

    if (rfconst != 2.0) begin
      $display("Expected real constant function value to be 2.0, actual is %f", rfconst);
      passed = 1'b0;
    end

    if (wfconstarg !== 1'b1) begin
      $display("Expected wire constant arg function value to be 1'b1, actual is %b", wfconstarg);
      passed = 1'b0;
    end

    if (wdfconstarg !== 1'b0) begin
      $display("Expected wire decl constant arg function value to be 1'b0, actual is %b", wdfconstarg);
      passed = 1'b0;
    end

    if (cfunc() !== 1'b0) begin
      $display("Expected constant function value to be 1'b0, actual is %b", cfunc());
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end

endmodule
