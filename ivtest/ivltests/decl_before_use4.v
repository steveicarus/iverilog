module test();

initial begin
  @(e);
  $display("used before declaration");
end

event e;

initial ->e;

endmodule
