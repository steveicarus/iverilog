// Check array word cannot be both procedurally and continuously assigned.
module test();

logic [7:0] p[1:0];
logic [7:0] q[1:0];

assign p[1] = 8'd2;

assign q = p;

integer i;

initial begin
  p[0] = 8'd3;
  p[1] = 8'd4;
  p[i] = 8'd5;
  q[0] = 8'd6;
  q[1] = 8'd7;
end

endmodule
