module counter(out, clk, reset);

  parameter WIDTH = 8;

  output [WIDTH-1 : 0] out;
  input            clk, reset;

  reg [WIDTH-1 : 0]   out;
  wire         clk, reset;

(* ivl_synthesis_on *)
  always @(posedge clk)
    out <= out + 1;

  always @(posedge reset)
    assign out = 0;

  always @(negedge reset)
    deassign out;
(* ivl_synthesis_off *)

initial $display("PASSED");

endmodule // counter
