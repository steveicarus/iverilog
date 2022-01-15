/*
 * This module instantiates the fa4 entity, which in turn
 * instantiates other entities. This demonstrates hierarchical
 * constructs in VHDL.
 */
module test;

reg [3:0] a, b;
reg cin;

wire [3:0] s;
wire cout;

initial begin
  cin = 0;
  a = 4'h2;
  b = 4'h3;
end

initial begin
  #1;
  if (s !== 4'h5) begin
     $display("Error in trivial sum");
     $finish;
   end
   $display ("PASSED");
end

fa4 duv (.c_i(cin), .va_i(a), .vb_i(b), .vs_o(s), .c_o(cout) );

endmodule // test
