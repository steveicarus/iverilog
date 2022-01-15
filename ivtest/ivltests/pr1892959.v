module top;
  reg pass = 1'b1;
  reg [1:0] in;
  wire out;

  function IS_NOT_ZERO;
    input [3:0] in;
    begin
      IS_NOT_ZERO = |in;
    end
  endfunction

  assign out = (IS_NOT_ZERO(in) == 1'b1);

  initial begin
    in = 2'b00;
    #1 if (out != 1'b0) begin
      $display("Failed for 2'b00 case.");
      pass = 1'b0;
    end

    in = 2'b01;
    #1 if (out != 1'b1) begin
      $display("Failed for 2'b01 case.");
      pass = 1'b0;
    end

    in = 2'b10;
    #1 if (out != 1'b1) begin
      $display("Failed for 2'b01 case.");
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
