`define my_macro(a0) \
  `define hello_``a0 $display("Hello %s", `"a0`")

module test();

initial begin
  `my_macro(world);
  `hello_world;
end

endmodule
