// A zero value MCD should be allowed, but does it process the arguments?
module top;
  integer mcd, test;

  initial begin
    test = 0;
    // Skipped, but function is called.
    #1;
    $display($stime);
    mcd = 0;
    $fstrobe(mcd, "The $fstrobe(%d, ...) ran.", mcd);
    $fdisplay(mcd, "The result for $fdisplay(%d, ...) is %d", mcd, my_func(1));
    $fwrite(mcd, "The result for $fwrite(%d, ...) is %d\n", mcd, my_func(1));

    #1;
    $display($stime);
    mcd = 1;
    $fstrobe(mcd, "The $fstrobe(%d, ...) ran.", mcd);
    $fdisplay(mcd, "The result for $fdisplay(%d, ...) is %d", mcd, my_func(1));
    $fwrite(mcd, "The result for $fwrite(%d, ...) is %d\n", mcd, my_func(1));
    $fflush(mcd);

    // Skipped, but function is called.
    #1;
    $display($stime);
    mcd = 0;
    $fstrobe(mcd, "The $fstrobe(%d, ...) ran.", mcd);
    $fdisplay(mcd, "The result for $fdisplay(%d, ...) is %d", mcd, my_func(1));
    $fwrite(mcd, "The result for $fwrite(%d, ...) is %d\n", mcd, my_func(1));

    #1;
    $display($stime);
    if (test != 6) $display("FAILED side effect test");
    else $display("PASSED");
  end

  function integer my_func;
    input incr;
    begin
      test = test + incr;
      my_func = test;
    end
  endfunction
endmodule
