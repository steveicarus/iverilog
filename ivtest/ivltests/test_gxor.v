// This module generate M single 2*HW-1 bit vector each T time steps

module stimulus #(parameter HW = 4, T = 10, M = 200) (
                 output reg [2*HW-1:0] a
                 );

int i;
int MAX;


initial begin
  MAX = 1 << 2*HW;
  for (i = 0; i < M; i=i+1) begin
    a = $random % MAX ;
    #T;
  end

end


endmodule

// This module always checks that y complies with an XOR reduction operation on 2*HW-1 bits input as x

module check #(parameter HW = 4) (input [2*HW-1:0] x, input y);

wire yi = ^x;

always @(y, yi) begin
  #1;
  if (y !== yi) begin
    $display("ERROR");
    $finish;
    end
end

endmodule


module test;
  parameter M = 200;  // number of test vectors
  parameter T = 10;   // time step unit
  parameter HW = 4;   // bit width of input vecotrs
  parameter S = M*T + 40;

  wire [2*HW-1:0] a;
  wire y;

  stimulus #(HW, T, M) stim  (.a(a));
  gxor_reduce  #(HW)   duv   (.a(a), .ar(y));
  check                check (.x(a), .y(y) );

  initial begin
    #S;
    $display("PASSED");
    $finish;
  end

endmodule
