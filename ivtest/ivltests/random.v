module test;

integer i;

initial begin
  for (i = 0; i < 1000; i++) begin
    $display("%h", $random);
  end
end

endmodule
