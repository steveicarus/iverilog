module top;
  string str_in = "Hello World!";
  int str_err;

  initial begin
      // This does not take arguments
    str_err = str_in.len(1'b1);

    $display("FAILED: should have elaboration errors!");
  end

endmodule
