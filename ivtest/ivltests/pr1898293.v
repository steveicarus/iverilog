module top;
  reg pass = 1'b1;

  reg [1:0] rval = 2'b10;
  wire [1:0] wval = (wval > 0) ? 2'b01 : 2'b00;
  // This works as follows:
  //   rlval starts are 0.0 which is not greater than 0.0 (false).
  //   This sets rlval to 2.0 which is greater than 0.0 (true).
  //   This then sets the value to 1.0 which is still true and stable.
  wire real rlval = (rlval > 0.0) ? 1.0 : 2.0;

  initial begin
    #1;
    if (rval != 2'b10) begin
      $display("FAILED initial value expected 2'b10, got %b.", rval);
      pass = 1'b0;
    end

    if (wval !== 2'b0x) begin
      $display("FAILED net value expected 2'b0x, got %b.", wval);
      pass = 1'b0;
    end

    if (rlval != 1.0) begin
      $display("FAILED net real value expected 1.0, got %f.", rlval);
      pass = 1'b0;
    end

    #1 assign rval = (rval > 0) ? 2'b01 : 2'b00;
    if (rval != 2'b01) begin
      $display("FAILED forced value expected 2'b01, got %b.", rval);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
