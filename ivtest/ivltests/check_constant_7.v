module top_module(
  input  wire [2:0] N,
  input  wire [7:0] In,
  output reg  [7:0] Out
);

wire [7:N] Array[7:0];

assign Array[0][0]   = In[0];
assign Array[0][7:1] = In[7:1];

initial begin
  Out[0]   = Array[0][0];
  Out[7:1] = Array[0][7:1];
end

endmodule
