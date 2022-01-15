(* this_is_module_bar *)
module bar(clk, rst, inp, out);
  input  wire clk;
  input  wire rst;
  input  wire inp;
  output reg  out;

  always @(posedge clk)
    if (rst) out <= 1'd0;
    else     out <= ~inp;

endmodule

(* this_is_module_foo *)
module foo(clk, rst, inp, out);
  input  wire clk;
  input  wire rst;
  input  wire inp;
  output wire out;

  bar bar_instance (clk, rst, inp, out);

  initial begin
    $display("PASSED");
  end

endmodule

