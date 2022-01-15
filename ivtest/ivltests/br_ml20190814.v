`timescale 1ns/1ps

module BUFGCE(
  output O,
  input  I,
  input  CE
);

bufif1(O, I, CE);

specify
  (I  => O) = (0.1, 0.2);
  (CE => O) = (0.3, 0.4);
endspecify

endmodule

module dut(
  output out,
  input  in,
  input  en
);

BUFGCE clk_IBUF_BUFG_inst(.O(out), .I(in), .CE(en));

endmodule

module top;

wire out;
reg in, en;

dut dut(out, in, en);

initial begin
  $sdf_annotate("ivltests/br_ml20190814.sdf", dut);
end

endmodule
