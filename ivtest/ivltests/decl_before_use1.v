module test();

initial begin
  v = 1;
  $display("%b", v);
  $display("used before declaration");
end

reg v;

endmodule
