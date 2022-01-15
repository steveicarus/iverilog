module test;

int count = 0;

initial begin
  for (int i = 0; i < 10; i++) begin
    for(int j = 0; j < 10; j++) begin
      count += 1;
    end
  end
  if (count === 100)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
