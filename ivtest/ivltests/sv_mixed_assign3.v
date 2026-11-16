// Check compound assignments with continuous drivers on other bits.

module test;

  logic [7:0] value;
  reg [3:0] drive = 3;

  assign value[7:4] = drive;

  initial begin
    value[3:0] = 1;
    value[3:0] += 1;
    #1 drive = 4;
    #1;
    if (value !== 8'h42) begin
      $display("FAILED: expected 42, got %h", value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
