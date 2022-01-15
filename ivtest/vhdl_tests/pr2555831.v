module top ();
  reg [31:0] din;
  wire [31:0] dout;

  test t(din, dout);

  initial begin
    din = 5;
    #1;
    $display("dout=%d", dout);
    if (dout == 5)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule // top

module test ( din, dout);
  input [31:0] din;
  output [31:0] dout;
  buff #(1) d0_1 ( .in(din[0:0]), .out(dout[0:0]));
  buff #(32) d0_32 ( .in(din[31:0]), .out(dout[31:0]));
endmodule // test

module buff (out, in);
  parameter SIZE=1;
  output [SIZE-1:0] out;
  input [SIZE-1:0]  in;
  assign out[SIZE-1:0] = in[SIZE-1:0];
endmodule // buff
