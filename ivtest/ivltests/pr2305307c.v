module top;
  reg passed;
  parameter zero = 1'b0;
  parameter one = 1'b1;
  parameter highz = 1'bz;
  parameter undef = 1'bx;

  initial begin
    passed = 1'b1;

    if (&zero !== 1'b0) begin
      $display("FAILED const. reduction & with input 1'b0, expected 1'b0, ",
               " got %b", &zero);
      passed = 1'b0;
    end
    if (&one !== 1'b1) begin
      $display("FAILED const. reduction & with input 1'b1, expected 1'b1, ",
               " got %b", &one);
      passed = 1'b0;
    end
    if (&highz !== 1'bx) begin
      $display("FAILED const. reduction & with input 1'bz, expected 1'bx, ",
               " got %b", &highz);
      passed = 1'b0;
    end
    if (&undef !== 1'bx) begin
      $display("FAILED const. reduction & with input 1'bx, expected 1'bx, ",
               " got %b", &undef);
      passed = 1'b0;
    end

    if (|zero !== 1'b0) begin
      $display("FAILED const. reduction | with input 1'b0, expected 1'b0, ",
               " got %b", |zero);
      passed = 1'b0;
    end
    if (|one !== 1'b1) begin
      $display("FAILED const. reduction | with input 1'b1, expected 1'b1, ",
               " got %b", |one);
      passed = 1'b0;
    end
    if (|highz !== 1'bx) begin
      $display("FAILED const. reduction | with input 1'bz, expected 1'bx, ",
               " got %b", |highz);
      passed = 1'b0;
    end
    if (|undef !== 1'bx) begin
      $display("FAILED const. reduction | with input 1'bx, expected 1'bx, ",
               " got %b", |undef);
      passed = 1'b0;
    end

    if (^zero !== 1'b0) begin
      $display("FAILED const. reduction ^ with input 1'b0, expected 1'b0, ",
               " got %b", ^zero);
      passed = 1'b0;
    end
    if (^one !== 1'b1) begin
      $display("FAILED const. reduction ^ with input 1'b1, expected 1'b1, ",
               " got %b", ^one);
      passed = 1'b0;
    end
    if (^highz !== 1'bx) begin
      $display("FAILED const. reduction ^ with input 1'bz, expected 1'bx, ",
               " got %b", ^highz);
      passed = 1'b0;
    end
    if (^undef !== 1'bx) begin
      $display("FAILED const. reduction ^ with input 1'bx, expected 1'bx, ",
               " got %b", ^undef);
      passed = 1'b0;
    end

    if (~&zero !== 1'b1) begin
      $display("FAILED const. reduction ~& with input 1'b0, expected 1'b1, ",
               " got %b", ~&zero);
      passed = 1'b0;
    end
    if (~&one !== 1'b0) begin
      $display("FAILED const. reduction ~& with input 1'b1, expected 1'b0, ",
               " got %b", ~&one);
      passed = 1'b0;
    end
    if (~&highz !== 1'bx) begin
      $display("FAILED const. reduction ~& with input 1'bz, expected 1'bx, ",
               " got %b", ~&highz);
      passed = 1'b0;
    end
    if (~&undef !== 1'bx) begin
      $display("FAILED const. reduction ~& with input 1'bx, expected 1'bx, ",
               " got %b", ~&undef);
      passed = 1'b0;
    end

    if (~|zero !== 1'b1) begin
      $display("FAILED const. reduction ~| with input 1'b0, expected 1'b1, ",
               " got %b", ~|zero);
      passed = 1'b0;
    end
    if (~|one !== 1'b0) begin
      $display("FAILED const. reduction ~| with input 1'b1, expected 1'b0, ",
               " got %b", ~|one);
      passed = 1'b0;
    end
    if (~|highz !== 1'bx) begin
      $display("FAILED const. reduction ~| with input 1'bz, expected 1'bx, ",
               " got %b", ~|highz);
      passed = 1'b0;
    end
    if (~|undef !== 1'bx) begin
      $display("FAILED const. reduction ~| with input 1'bx, expected 1'bx, ",
               " got %b", ~|undef);
      passed = 1'b0;
    end

    if (~^zero !== 1'b1) begin
      $display("FAILED const. reduction ~^ with input 1'b0, expected 1'b1, ",
               " got %b", ~^zero);
      passed = 1'b0;
    end
    if (~^one !== 1'b0) begin
      $display("FAILED const. reduction ~^ with input 1'b1, expected 1'b0, ",
               " got %b", ~^one);
      passed = 1'b0;
    end
    if (~^highz !== 1'bx) begin
      $display("FAILED const. reduction ~^ with input 1'bz, expected 1'bx, ",
               " got %b", ~^highz);
      passed = 1'b0;
    end
    if (~^undef !== 1'bx) begin
      $display("FAILED const. reduction ~^ with input 1'bx, expected 1'bx, ",
               " got %b", ~^undef);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
