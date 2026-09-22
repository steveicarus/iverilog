module top;
  parameter string param_in  = "Hello World!";
  parameter int param_err = param_in.len(1'b1);

  initial $display("FAILED: should have elaboration errors!");
endmodule
