// Check that parameters without default values outside the parameter port list
// generate an error.

module a;

parameter A;

initial begin
  $display("FAILED");
end

endmodule

module test;
  a #(.A(10)) i_a();
endmodule
