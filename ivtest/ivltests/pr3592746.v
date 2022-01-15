module pr3592746();

reg     Iteration;

integer RepeatCount[1:0];

task RepeatTest;

begin
  repeat(Iteration == 1 ? 3 : 2) begin
    $display("Iteration = %b", Iteration);
    RepeatCount[Iteration] = RepeatCount[Iteration] + 1;
  end
end

endtask

initial begin
  RepeatCount[0] = 0;
  RepeatCount[1] = 0;
  Iteration = 0;
  RepeatTest;
  Iteration = 1;
  RepeatTest;
  if ((RepeatCount[0] == 2) && (RepeatCount[1] == 3))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
