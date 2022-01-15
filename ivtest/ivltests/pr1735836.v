module top;
  wire [1:0] out;
  reg [1:0] in1, in2;

  initial begin
    $monitor($time,,"out=%d", out);
    in1 = 2'd1;
    in2 = 2'd2;
    #1 force out = in1;
    #1 force out = in2;
    #1 release out;
  end
endmodule
