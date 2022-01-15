/*
 * There are a number of problem that this example uncovers.
 *
 * It appears that the inverter connected to the ctl input of the
 * tranif gate is not getting the signal passed to it. It looks
 * like once the ctl signal is pulled into the island it can not
 * propagate the signal back out. The fix may be in the compiler
 * where we should only use the island port signal for the tranif
 * control instead of any signal that connects to the ctl net.
 *
 * When the ctl signal removes the connection between the two
 * nets they should self resolve. Is appears that when the ctl
 * signal is removed the nets stay at their current value.
 */
module top;
  reg pass;
  reg ctl, ctl2, in, in2;
  wire y1, y2, ctlb, y2b;

  assign y1 = in;
  pullup (weak1) (y2);

  tranif0 q1(y1, y2, ctl);

  assign y2 = ctl2 ? in2 : 1'bz;

  not q2(ctlb, ctl);
  not q3(y2b, y2);

  initial begin
    pass = 1'b1;
    // The tran gate is closed and both sides should track 'in'.
    ctl = 1'b0;
    ctl2 = 1'b0;
    in = 1'b1;
    #1;
    if (ctlb !== 1'b1) begin
      $display("Failed ctlb with ctl = 0, expected 1'b1, got %b", ctlb);
      pass = 1'b0;
    end
    if (y2 !== 1'b1) begin
      $display("Failed tran with ctl = 0, in = 1, expected 1'b1, got %b", y2);
      pass = 1'b0;
    end
    if (y2b !== 1'b0) begin
      $display("Failed y2b with ctl = 0, in = 1, expected 1'b0, got %b", y2b);
      pass = 1'b0;
    end
    in = 1'b0;
    #1;
    if (y2 !== 1'b0) begin
      $display("Failed tran with ctl = 0, in = 0, expected 1'b0, got %b", y2);
      pass = 1'b0;
    end
    if (y2b !== 1'b1) begin
      $display("Failed y2b with ctl = 0, in = 0, expected 1'b1, got %b", y2b);
      pass = 1'b0;
    end

    // The tran gate is open so y2 should go high (pullup).
    ctl = 1'b1;
    #1;
    if (ctlb !== 1'b0) begin
      $display("Failed ctlb with ctl = 1, expected 1'b0, got %b", ctlb);
      pass = 1'b0;
    end
    if (y2 !== 1'b1) begin
      $display("Failed tran with ctl = 1, expected 1'b1, got %b", y2);
      pass = 1'b0;
    end
    if (y2b !== 1'b0) begin
      $display("Failed y2b with ctl = 1, expected 1'b0, got %b", y2b);
      pass = 1'b0;
    end

    // Now try driving y2 from in2.
    ctl2 = 1'b1;
    in2 = 1'b1;
    #1;
    if (y2 !== 1'b1) begin
      $display("Failed tran with ctl2 = 1, in2 = 1, expected 1'b1, got %b", y2);
      pass = 1'b0;
    end
    if (y2b !== 1'b0) begin
      $display("Failed y2b with ctl2 = 1, in2 = 1, expected 1'b0, got %b", y2b);
      pass = 1'b0;
    end
    in2 = 1'b0;
    #1;
    if (y2 !== 1'b0) begin
      $display("Failed tran with ctl2 = 1, in2 = 0, expected 1'b0, got %b", y2);
      pass = 1'b0;
    end
    if (y2b !== 1'b1) begin
      $display("Failed y2b with ctl2 = 1, in2 = 0, expected 1'b1, got %b", y2b);
      pass = 1'b0;
    end

    // Now back to just a pullup on y2.
    ctl2 = 1'b0;
    #1;
    if (y2 !== 1'b1) begin
      $display("Failed tran with ctl2 = 0, expected 1'b1, got %b", y2);
      pass = 1'b0;
    end
    if (y2b !== 1'b0) begin
      $display("Failed y2b with ctl2 = 0, expected 1'b0, got %b", y2b);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
