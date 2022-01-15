module top;
  integer ival;
  real rval;
  initial begin
    $display("--- Printing as real ---");
    $display("1/0 is %f. (Should be 0 -- x prints as 0)", 1/0);
    $display("1/0.0 is %f. (Should be inf)", 1/0.0);
    $display("1.0/0 is %f. (Should be inf)", 1.0/0);
    $display("1.0/0.0 is %f. (should be inf)", 1.0/0.0);

    // Moving these two lines before the previous four lines makes 1/0 print
    // a large number, but not inf!
    rval = 0.0;
    ival = 0;
    $display("1/integer zero is %f. (Should be 0 -- x prints as 0)", 1/ival);
    $display("1/real zero is %f. (should be inf)", 1/rval);
    $display("1.0/integer zero is %f. (Should be inf)", 1.0/ival);
    $display("1.0/real zero is %f.", 1.0/rval);

    $display("\n--- Printing as integer ---");
    $display("1/0 is %d (Should be x)", 1/0);
    $display("1/0.0 is %d", 1/0.0);
    $display("1.0/0 is %d", 1.0/0);
    $display("1.0/0.0 is %d", 1.0/0.0);

    $display("1/integer zero is %d. (Should be x)", 1/ival);
    $display("1/real zero is %d.", 1/rval);
    $display("1.0/integer zero is %d.", 1.0/ival);
    $display("1.0/real zero is %d.", 1.0/rval);
  end
endmodule
