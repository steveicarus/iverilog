module test();

initial begin
  $display("%b", w);
  $display("used before declaration");
end

localparam w = 8'hAA;

endmodule
