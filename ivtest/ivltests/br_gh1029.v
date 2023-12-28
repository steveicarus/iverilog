module test;
  if (1) begin
    $error("This is before the fatal so it should display.");
    $fatal(0, "This should be a fatal elaboration error.");
    $error("This should not display since is after the fatal.");
  end
  initial $fatal(0, "FAILED elaboration assertion.");
endmodule
