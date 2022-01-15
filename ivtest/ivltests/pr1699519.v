module top;
  real rval;
  reg [7:0] rgval;

  initial begin
    rgval = 8'ha5;
    rval = 1234567890;

    $display("Checking h and H: %h, %H", rgval, rgval);
    $display("Checking x and X: %x, %X", rgval, rgval);
    $display("Checking g and G: %g, %G", rval, rval);
    $display("Checking e and E: %e, %E", rval, rval);
  end
endmodule
