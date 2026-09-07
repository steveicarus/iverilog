// Check that an instance list can mix omitted and empty port lists.

module M;
  localparam VALUE = 1;
endmodule

module test;
  M first [1:0], middle(), last [1:0];

  initial begin
    if (first[0].VALUE !== 1 || first[1].VALUE !== 1 ||
        middle.VALUE !== 1 || last[0].VALUE !== 1 || last[1].VALUE !== 1)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
