module top;

  initial begin
    if ("this matches" == "this\
 matches") $display("PASSED");
    else $display("FAILED");
  end

endmodule
