`timescale 1ns/1ps

module Buffer(output Y, input A);

  specparam Delay = 0.1;

  assign #Delay Y = A;

endmodule

module Buffer1(output Y, input A);
  Buffer inst(.Y(Y), .A(A));
endmodule

module Buffer2(output Y, input A);
  Buffer inst(.Y(Y), .A(A));
endmodule

`timescale 1ps/1ps

module dut();

reg  n1;
wire n2;
wire n3;

Buffer1 b1(.A(n1), .Y(n2));
Buffer2 b2(.A(n2), .Y(n3));

initial begin
  $monitor("%t %b %b %b", $time, n1, n2, n3);
  #1000 n1 = 0;
  #1000 n1 = 1;
  #1000 $finish(0);
end

endmodule
