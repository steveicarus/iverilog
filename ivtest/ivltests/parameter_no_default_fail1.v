// Check that not providing a value during module instantiation for a parameter
// without a default value generates an error.

module a #(
  parameter A
);

initial begin
  $display("FAILED");
end

endmodule

module test;
  a i_a();
endmodule
