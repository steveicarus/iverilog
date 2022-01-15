`define STOP_HERE
`define my_macro(a0, a1=HERE, a2=HERE) \
  `ifdef STOP_``a1 \
    $display(a0);  \
  `elsif STOP_``a2 \
    $display(a0, a1); \
  `else \
    $display(a0, a1, a2); \
  `endif \

module test();

initial begin
  `my_macro("No args");
  `my_macro("Args = %s", "hello");
  `my_macro("Args = %0d, %s", 123, "hello");
end


endmodule
