module top;
  reg pass;
  reg in;
  reg pout;
  wire out;

  function invert;
    input in;
    // When this is a named block the compiler creates a fork/join to
    // create the new scope. The problem with this is that of_EXEC_UFUNC
    // opcode does not work correctly since the vthread_run(child) call
    // returns when the join is executed which then copies the return value
    // before the function body code has actually run. This causes the
    // results to be delayed by one call. Does this need to be split into
    // two functions. One that acts like fork and copies the input values
    // and one that acts like join and returns the function result?
    // It appears that procedural user function calls work correctly since
    // they use fork/join to call the user function.
    // Both V0.9 and development have this problem.
    begin: block_name
      invert = ~in;
      $display("Function should return %b when given %b.", invert, in);
    end
  endfunction

  assign out = invert(in);

  initial begin
    pass = 1'b1;

    in = 1'b0;
    #1;
    if (out !== 1'b1) begin
      $display("CA result was %b when given %b, expect 1'b1.", out, in);
      pass = 1'b0;
    end
    pout = invert(in);
    if (pout !== 1'b1) begin
      $display("Result was %b when given %b, expect 1'b1.", pout, in);
      pass = 1'b0;
    end

    in = 1'b1;
    #1;
    if (out !== 1'b0) begin
      $display("CA result was %b when given %b, expect 1'b0.", out, in);
      pass = 1'b0;
    end
    pout = invert(in);
    if (pout !== 1'b0) begin
      $display("Result was %b when given %b, expect 1'b0.", pout, in);
      pass = 1'b0;
    end

    in = 1'bz;
    #1;
    if (out !== 1'bx) begin
      $display("CA result was %b when given %b, expect 1'bx.", out, in);
      pass = 1'b0;
    end
    pout = invert(in);
    if (pout !== 1'bx) begin
      $display("Result was %b when given %b, expect 1'bx.", pout, in);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
