/* pr1634526.v */

module test;
  initial begin
    $display("Working: This (%0d) should be 255.", minus1(256));
// This crashes!
     $display("Broken:  This (%0d) should be 255", minus1(2**8));
// And this gives the wrong result!
     $display("Broken:  This (%0d) should be 255.", minus1(2**8-1+1));
     $display("         started with %0d.", 2**8-1+1);
    $finish(0);
  end

  function integer minus1;
    input value;
    integer value;
    minus1 = value - 1;
  endfunction
endmodule
