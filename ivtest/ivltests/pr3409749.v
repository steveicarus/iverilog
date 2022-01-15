module top;
  reg pass = 1'b1;

  genvar lp;
  for (lp=1; lp <= 128; lp = lp + 1) begin: loop
    test #(lp) dut();
  end

  initial #1000 if (pass) $display("PASSED");
endmodule

module test;
  parameter wid = 62;

  localparam X = {4'b1000, {wid{1'b0}}};
  localparam Y = {1'b1, {wid{1'b0}}};

  reg [wid:0] y =  Y;
  reg [wid+3:0] x =  X;
  reg [3:0] res;

  initial begin
    #wid; // Wait for the x and y values to get assigned.
    res = X/Y;
    if (res !== 4'b1000) begin
      $display("Failed const. division for    %3d, expected 4'b1000, got %b",
                wid, res);
      top.pass = 1'b0;
    end
    res = X/y;
    if (res !== 4'b1000) begin
      $display("Failed const. numerator for   %3d, expected 4'b1000, got %b",
                wid, res);
      top.pass = 1'b0;
    end
    res = x/Y;
    if (res !== 4'b1000) begin
      $display("Failed const. denominator for %3d, expected 4'b1000, got %b",
                wid, res);
      top.pass = 1'b0;
    end
    res = x/y;
    if (res !== 4'b1000) begin
      $display("Failed variable division for  %3d, expected 4'b1000, got %b",
                wid, res);
      top.pass = 1'b0;
    end
  end
endmodule
