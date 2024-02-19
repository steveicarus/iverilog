module test();

initial begin
  $display("%b", w);
  $display("FAILED");
end

localparam w = 8'hAA;

endmodule
