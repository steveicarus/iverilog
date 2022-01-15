module test (
  input rst
);

  reg [31:0]   a[99-1:35];
  reg [5:0]  b;

  always @(b) begin
    a[35 + (b<<3)] <= #1 a[35 + (b<<3)] + 32'd1;
  end

  initial begin
    $monitor(a[35],, a[43]);
    a[35] = 32'd0;
    a[43] = 32'd8;
    b = 0;
    #2 b = 1;
    #2 b = 0;
    #2 b = 1;
    #2 b = 0;
  end
endmodule
