module top;
  string str_in = "Hello World!";
  string str_err;

  initial begin
      // This does not take arguments
    str_err = str_in.tolower(1'b1);

    $display("FAILED: should have elaboration errors!");
  end

endmodule
