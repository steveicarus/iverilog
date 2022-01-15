module top;
  reg pass = 1'b1;

  parameter one = 1'b1;
  parameter zero = 1'b0;
  parameter udef = 1'bx;

  reg [2:0] four = 3'd4;
  reg [2:0] five = 3'd5;
  reg [2:0] six = 3'd6;
  reg [2:0] seven = 3'd7;

  wire real rl1 = one  ? four : 4.5;     // 4.0
  wire real rl2 = zero ? 4.0 : five;     // 5.0
  wire real rl3 = udef ? six : 6.0;      // 6.0
  wire real rl4 = udef ? seven : seven;  // 7.0

  initial begin
    #1;
    if (rl1 != 4.0) begin
      $display("FAILED: real expression one, expected 4.0, got %f", rl1);
      pass = 1'b0;
    end

    if (rl2 != 5.0) begin
      $display("FAILED: real expression two, expected 4.0, got %f", rl2);
      pass = 1'b0;
    end

    if (rl3 != 6.0) begin
      $display("FAILED: real expression three, expected 4.0, got %f", rl3);
      pass = 1'b0;
    end

    if (rl4 != 7.0) begin
      $display("FAILED: real expression four, expected 7.0, got %f", rl4);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
