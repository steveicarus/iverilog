module test;

reg [7:0] a;
reg [7:0] b;
reg [7:0] c;

initial begin
  c = a ~& b;
  c = a ~| b;
end

endmodule
