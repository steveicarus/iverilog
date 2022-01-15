module top;
  reg [40*8-1:0] result;
  integer iindex [1:0];
  reg signed [15:0] rindex [1:0];
  wire signed [15:0] windex [1:0];

  assign windex[0] = -1;

  initial begin
    iindex[0] = -1;
    rindex[0] = -1;
    #1;
    $display("iindex[0] = %0d", iindex[0]);
    $display("rindex[0] = %0d", rindex[0]);
    $display("windex[0] = %0d", windex[0]);
  end
endmodule
