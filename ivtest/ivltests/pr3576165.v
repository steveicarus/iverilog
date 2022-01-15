module top;
  reg pass;
  enum reg[0:0] { IDLE = 1'b0,
                  BUSY = 1'b1
                } state, next;

  initial begin
    pass = 1'b1;
    next = IDLE;
    if (state !== 1'bx) begin
      $display("FAILED initial state, got %b", state);
      pass = 1'b0;
    end
    #1;
    state = next;
    if (state !== 1'b0) begin
      $display("FAILED idle state, got %b", state);
      pass = 1'b0;
    end
    next = BUSY;
    #1;
    state <= next;
    if (state !== 1'b0) begin
      $display("FAILED still idle state, got %b", state);
      pass = 1'b0;
    end
    #1;
    if (state !== 1'b1) begin
      $display("FAILED busy state, got %b", state);
      pass = 1'b0;
    end
    if (pass) $display("PASSED");
  end

endmodule
