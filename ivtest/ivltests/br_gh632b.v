module tb();

reg inputs[1:0];
reg out;

always @* begin
  if (inputs[1]) out = inputs[0];
end

reg failed;

(* ivl_synthesis_off *)
initial begin
  failed = 0;

  #1 inputs[1] = 1;
  #1 inputs[0] = 0;
  #1 $display(inputs[1],,inputs[0],,out);
  if (out !== 0) failed = 1;
  #1 inputs[1] = 0;
  #1 inputs[0] = 1;
  #1 $display(inputs[1],,inputs[0],,out);
  if (out !== 0) failed = 1;
  #1 inputs[1] = 1;
  #1 $display(inputs[1],,inputs[0],,out);
  if (out !== 1) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
