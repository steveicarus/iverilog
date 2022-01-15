`define OPT1_DISPLAY $display("opt1");
`define OPT2_DISPLAY $display("opt2");

`define INDIRECT_OPT(OPTN) ```OPTN``_DISPLAY


module t;

  initial begin
    `INDIRECT_OPT(OPT1)
    `INDIRECT_OPT(OPT2)
  end
endmodule
