module bar(clk, rst, inp, out);
  input  wire clk;
  input  wire rst;
  input  wire inp;
  output reg  out;

  always @(posedge clk)
    if (rst) out <= 1'd0;
    else     out <= ~inp;

endmodule

module foo(clk, rst, inp, out_a, out_b);
  input  wire clk;
  input  wire rst;
  input  wire inp;
  output wire out_a;
  output wire out_b;

  bar bar_instance_1 ( (* this_is_clock *) .clk(clk), .rst(rst), .inp(inp), .out(out_a) );
  bar bar_instance_2 ( clk, (* this_is_reset *) rst, inp, out_b );

  initial begin
    $display("PASSED");
  end

endmodule
