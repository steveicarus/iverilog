`ifndef MACRO1
`define MACRO1 21
`endif
`ifndef MACRO2
`define MACRO2 22
`endif
`ifndef MACRO3
`define MACRO3 23
`endif

module test2(input w);

buf(a,b);

initial #2 begin
  $display("test2 macro1 = %0d", `MACRO1 );
  $display("test2 macro2 = %0d", `MACRO2 );
  $display("test2 macro3 = %0d", `MACRO3 );
  $display("test2 wire = %b", a);
end

endmodule
