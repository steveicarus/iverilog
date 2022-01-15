module test;

reg [15:0] x;
reg [15:0] y;

initial begin
  x = 0;
  y = 0;
  $monitor(x,,y);
  $background_copy(x, y);
  #1 $display("started background copy");
  #1 x = 1;
  #1 x = 2;
  #1 $display("finished background copy");
end

endmodule
