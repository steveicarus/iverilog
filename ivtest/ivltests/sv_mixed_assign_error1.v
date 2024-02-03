// Check entire array cannot be both procedurally and continuously assigned.
module test();

logic [7:0] p[1:0];
logic [7:0] q[1:0];

assign q = p;

initial begin
  q = '{ 8'd0, 8'd0 };
end

endmodule
