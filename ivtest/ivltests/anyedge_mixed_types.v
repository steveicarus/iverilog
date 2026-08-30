module anyedge_mixed_types;
  reg [3:0] bits;
  real rval;
  string sval;
  integer hits;

  // All three inputs share an anyedge functor, but each input port keeps
  // state with the fixed type of its connected net.
  always @(bits or rval or sval)
    hits = hits + 1;

  initial begin
    bits = 0;
    rval = 0.0;
    sval = "";
    #1 hits = 0;

    bits = 1;
    #1 bits = 1;
    #1 rval = 1.5;
    #1 rval = 1.5;
    #1 sval = "one";
    #1 sval = "one";
    #1 bits = 2;
    #1 rval = -2.5;
    #1 sval = "two";
    #1;

    if (hits == 6)
      $display("PASSED");
    else
      $display("FAILED: hits=%0d", hits);
  end
endmodule
