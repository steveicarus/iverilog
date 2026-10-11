// Check that empty named parameter arguments do not assign a value.

module M #(parameter VALUE = 1);
endmodule

module test;

  M #(.VALUE(), .VALUE()) i_empty();
  M #(.VALUE(), .VALUE(2)) i_before();
  M #(.VALUE(3), .VALUE()) i_after();

  initial begin
    if (i_empty.VALUE == 1 && i_before.VALUE == 2 && i_after.VALUE == 3)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
