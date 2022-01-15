module top;
  reg pass = 1'b1;
  reg result;

  function freg;
    input reg in;
    freg = in;
  endfunction

  function fnreg(input reg in);
    fnreg = in;
  endfunction

  task toreg;
    output reg out;
    input reg in;
    out = in;
  endtask

  task tnoreg(output reg out, input reg in);
    out = in;
  endtask

  task tioreg;
    inout reg io;
    io = 1'b1;
  endtask

  task tnioreg(inout reg io);
    io = 1'b0;
  endtask

  initial begin
    result = freg(1'b1);
    if (result !== 1'b1) begin
      $display("FAILED: freg()");
      pass = 1'b0;
    end

    result = fnreg(1'b0);
    if (result !== 1'b0) begin
      $display("FAILED: fnreg()");
      pass = 1'b0;
    end

    toreg(result, 1'b1);
    if (result !== 1'b1) begin
      $display("FAILED: toreg()");
      pass = 1'b0;
    end

    tnoreg(result, 1'b0);
    if (result !== 1'b0) begin
      $display("FAILED: tnoreg()");
      pass = 1'b0;
    end

    tioreg(result);
    if (result !== 1'b1) begin
      $display("FAILED: tioreg()");
      pass = 1'b0;
    end

    tnioreg(result);
    if (result !== 1'b0) begin
      $display("FAILED: tnioreg()");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
