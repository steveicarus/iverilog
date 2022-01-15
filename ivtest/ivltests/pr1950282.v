module test;
  function integer fn (input integer a, b, input integer c);
    fn = ((a > b) ? a : b) + c;
  endfunction

  initial begin
    if (fn(2, 4, 3) != 7) $display("Failed");
    else $display("PASSED");
  end
endmodule
