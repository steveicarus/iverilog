`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

module top;
  reg pass;
  reg [5:0] cond;
  reg [2:1] expr;
  integer result;

  always @(cond or expr) begin
    casex (cond)
     6'b01_??10           : result = 1;
     {2'b10, 4'b??10}     : result = 2;
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
     {expr[1:0], 4'b??01} : result = 3;
     expr[11:6]           : result = 4;
`else
     {expr[1], 1'bx, 4'b??01} : result = 3;
     6'bxxxxxx                : result = 4;
`endif
     default              : result = 0;
   endcase
  end

  initial begin
    pass = 1'b1;

    expr = 2'b10;
    cond = 6'b01_xx10;
    #1 if (result != 1) begin
      $display("Failed case expr 1 test, got expr %0d", result);
      pass = 1'b0;
    end

    cond = 6'bxx_xxxx;
    #1 if (result != 1) begin
      $display("Failed case expr 1a test, got expr %0d", result);
      pass = 1'b0;
    end

    cond = 6'b10_zz10;
    #1 if (result != 2) begin
      $display("Failed case expr 2 test, got expr %0d", result);
      pass = 1'b0;
    end

    cond = 6'b0x_zz01;
    #1 if (result != 3) begin
      $display("Failed case expr 3 test, got expr %0d", result);
      pass = 1'b0;
    end

    cond = 6'b11_1111;
    #1 if (result != 4) begin
      $display("Failed case expr 1a test, got expr %0d", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
