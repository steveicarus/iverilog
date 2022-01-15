module top();
  reg [7:0] a, a_a, a__a;

  initial begin
    a = 1;
    #1;
    a_a = 2;
    #1;
    a__a = 3;
    #1;

    if (a == 1 && a_a == 2 && a__a == 3)
      $display("PASSED");
    else
      $display("FAILED");

    $finish;
  end

endmodule // top
