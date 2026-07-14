module ff_initial_driver;
  logic a;

  initial a = 1'b0;

  always_ff @(posedge a) begin
    a <= 1'b1;
  end
endmodule

module ff_always_driver;
  logic clk;
  logic q;

  always @* begin
    q = clk;
  end

  always_ff @(posedge clk) begin
    q <= 1'b1;
  end
endmodule

module ff_ff_driver;
  logic clk1;
  logic clk2;
  logic q;

  always_ff @(posedge clk1) begin
    q <= 1'b0;
  end

  always_ff @(posedge clk2) begin
    q <= 1'b1;
  end
endmodule
