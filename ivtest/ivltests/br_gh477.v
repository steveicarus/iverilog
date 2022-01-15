module top;
  logic passed;
  logic [7:0] y;
  logic [7:0] yv;
  logic [39:0] a;

  logic [3:0][9:0] t;
  int idx;

  assign t = a;

  assign y = t[1][9-:8];
  assign yv = t[1][idx-:8];

  initial begin
    passed = 1'b1;
    idx = 9;
    a = {10'h3ff, 10'h2a5, 10'h000};
    #1;
    if (y != 8'ha9) begin
      $display("FAILED: expected 8'ha9 for constant select, got %h." , y);
      passed = 1'b0;
    end
    if (yv != 8'ha9) begin
      $display("FAILED: expected 8'ha9 for variable select, got %h." , yv);
      passed = 1'b0;
    end

    if (passed)  $display("PASSED");
  end
endmodule
