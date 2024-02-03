// Check entire vector cannot be both procedurally and continuously assigned.
module test();

logic [7:0] p;
logic [7:0] q;

assign q = p;

initial begin
  q = 8'd0;
end

endmodule
