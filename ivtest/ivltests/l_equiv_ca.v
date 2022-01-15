module top;
  wire res, ler0, ler1, ler2, ler3;
  wire [1:0] lew, lews;
  real rval1, rval2;
  reg val1, val2;
  reg [3:0] wval1, wval2;
  reg pass;

  assign res = val1 <-> val2;
  assign lew = wval1 <-> wval2;
  assign lews = $signed(wval1 <-> wval2);
  assign ler0 = rval1 <-> val2;
  assign ler1 = val1 <-> rval2;
  assign ler2 = rval1 <-> val2;
  assign ler3 = rval1 <-> rval2;

  initial begin
    pass = 1'b1;

    val1 = 1'b0;
    val2 = 1'b0;
    #1;
    if (res !== 1'b1) begin
      $display("FAILED: 1'b0 <-> 1'b0 returned %b not 1'b1", res);
      pass = 1'b0;
    end
    val2 = 1'b1;
    #1;
    if (res !== 1'b0) begin
      $display("FAILED: 1'b0 <-> 1'b1 returned %b not 1'b0", res);
      pass = 1'b0;
    end
    val2 = 1'bx;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'b0 <-> 1'bx returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'bz;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'b0 <-> 1'bz returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val1 = 1'b1;
    val2 = 1'b0;
    #1;
    if (res !== 1'b0) begin
      $display("FAILED: 1'b1 <-> 1'b0 returned %b not 1'b0", res);
      pass = 1'b0;
    end
    val2 = 1'b1;
    #1;
    if (res !== 1'b1) begin
      $display("FAILED: 1'b1 <-> 1'b1 returned %b not 1'b1", res);
      pass = 1'b0;
    end
    val2 = 1'bx;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'b1 <-> 1'bx returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'bz;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'b1 <-> 1'bz returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val1 = 1'bx;
    val2 = 1'b0;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'b0 returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'b1;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'b1 returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'bx;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'bx returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'bz;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bx <-> 1'bz returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val1 = 1'bz;
    val2 = 1'b0;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'b0 returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'b1;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'b1 returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'bx;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'bx returned %b not 1'bx", res);
      pass = 1'b0;
    end
    val2 = 1'bz;
    #1;
    if (res !== 1'bx) begin
      $display("FAILED: 1'bz <-> 1'bz returned %b not 1'bx", res);
      pass = 1'b0;
    end

    rval1 = 0.0;
    val2 = 1'b0;
    #1;
    if (ler0 !== 1'b1) begin
      $display("FAILED: 0.0 <-> 1'b0 returned %b not 1'b1", ler0);
      pass = 1'b0;
    end
    val1 = 1'b0;
    rval2 = 2.0;
    #1;
    if (ler1 !== 1'b0) begin
      $display("FAILED: 1'b0 <-> 2.0 returned %b not 1'b0", ler1);
      pass = 1'b0;
    end
    rval1 = 2.0;
    val2 = 1'bx;
    #1;
    if (ler2 !== 1'bx) begin
      $display("FAILED: 2.0 <-> 1'bx returned %b not 1'bx", ler2);
      pass = 1'b0;
    end
    rval1 = -5.0;
    rval2 = 2.0;
    #1;
    if (ler3 !== 1'b1) begin
      $display("FAILED: -5.0 <-> -2.0 returned %b not 1'b1", ler3);
      pass = 1'b0;
    end
    wval1 = 4'b0110;
    wval2 = 4'b1001;
    #1;
    if (lew !== 2'b01) begin
      $display("FAILED: 4'b0110 <-> 4'b1001 returned %b not 2'b01", lew);
      pass = 1'b0;
    end
    if (lews !== 2'b11) begin
      $display("FAILED: 4'b0110 <-> 4'b1001 returned %b not 2'b11", lews);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
