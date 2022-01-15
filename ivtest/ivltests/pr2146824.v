module bug();

reg [9:0] i;
reg [7:0] j;

initial begin
  i = 10'h3ff;
  j = (i / 4) & 8'hfe;
  $display("'h%h", j);
end

endmodule
