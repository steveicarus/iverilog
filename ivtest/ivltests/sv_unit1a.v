`define MACRO2 12
`define MACRO3 13

`default_nettype tri1

module test1();

buf(a,b);

initial #1 begin
  $display("test1 macro1 = %0d", `MACRO1 );
  $display("test1 macro2 = %0d", `MACRO2 );
  $display("test1 macro3 = %0d", `MACRO3 );
  $display("test1 wire = %b", a);
end

endmodule

`undef MACRO1
