module test();

assign w = 1;

initial begin
  $display("%b", w);
  $display("FAILED");
end

wire [7:0] w;

endmodule
