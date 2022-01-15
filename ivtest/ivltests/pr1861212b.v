module top;
  wire [63:0] vo;

  rcvr U1(vo);
  drvr U2(vo);
endmodule

module rcvr(input wire [63:0] vo);

  always @(vo)
    $display("Real value is %f at %g", $bitstoreal(vo), $time);
endmodule

module drvr(output wire [63:0] vo);
  real vint;

  assign vo = $realtobits(vint);

  initial begin
    vint = 3.3;
    #1000 vint = 4.5776;
    #1000 vint = -4;
  end
endmodule
