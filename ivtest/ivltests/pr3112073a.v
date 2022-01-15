module top;
  reg real [1:0] a;

  initial begin
    a[0] = 0.3;
    a[1] = 0.4;
    $display("FAILED");
  end
endmodule
