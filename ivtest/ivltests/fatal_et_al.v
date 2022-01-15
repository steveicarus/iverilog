module top;
  initial begin
    #1 $info("This is the $info message.");
    #1 $warning("This is the $warning message.");
    #1 $error("This is the $error message.");
    #1 $display("Check that the messages are correct.");
  end
endmodule
