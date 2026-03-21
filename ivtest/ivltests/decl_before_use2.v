module test();

assign w = 1;

initial begin
  $display("%b", w);
  $display("used before declaration");
end

wire [7:0] w;

endmodule
