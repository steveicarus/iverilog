module test();

initial begin
  v = 1;
  $display("%b", v);
  $display("FAILED");
end

reg v;

endmodule
