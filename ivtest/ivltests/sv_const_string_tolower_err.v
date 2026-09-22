module top;
  parameter string param_in  = "Hello World!";
  parameter string param_err = param_in.tolower(1'b1);

  initial $display("FAILED: should have elaboration errors!");
endmodule
