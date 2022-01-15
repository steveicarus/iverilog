module top;
  reg pass;
  reg [5:0] cond;
  reg [2:1] expr;
  integer result;

  always @(cond or expr) begin
    casex (cond)
     6'b01_??10           : result = 1;
     {2'b10, 4'b??10}     : result = 2;
     {expr, 4'b??01}      : result = 3;
     {expr[2], 5'b0??11}  : result = 4;
     {expr[2:1], 4'b??11} : result = 5;
     {expr, 4'b??00}      : result = 6;
     default              : result = 0;
   endcase
  end

  initial begin
    pass = 1'b1;

    cond = 6'b01_xx10;
    #1;
    if (result != 1) begin
      $display("Failed case expr 1 test, got expr %0d", result);
      pass = 1'b0;
    end

    cond = 6'b10_zz10;
    #1;
    if (result != 2) begin
      $display("Failed case expr 2 test, got expr %0d", result);
      pass = 1'b0;
    end

    expr = 2'b1?;
    cond = 6'b1x_xx01;
    #1;
    if (result != 3) begin
      $display("Failed case expr 3 test, got expr %0d", result);
      pass = 1'b0;
    end

    expr = 2'b0z;
    cond = 6'b00_xx11;
    #1;
    if (result != 4) begin
      $display("Failed case expr 4 test, got expr %0d", result);
      pass = 1'b0;
    end

    expr = 2'b?1;
    cond = 6'bx1_xx11;
    #1;
    if (result != 5) begin
      $display("Failed case expr 5 test, got expr %0d", result);
      pass = 1'b0;
    end

    expr = 2'b11;
    cond = 6'b11_xx00;
    #1;
    if (result != 6) begin
      $display("Failed case expr 6 test, got expr %0d", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
