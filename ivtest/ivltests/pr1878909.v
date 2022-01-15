module test_top();

  wire [3:0] test_val;

  test_mod test_mod0 [3:0](
   .in_0(1'b1),
   .out_0(test_val)
  );

  initial begin
    #1;
    if(test_val != 4'b0000) $display("Failed");
    else $display("PASSED");
  end
endmodule

module test_mod(
   in_0, out_0
  );
  input  in_0;
  output out_0;

  function test;
  input moo;
  begin
    test = ~moo;
  end
  endfunction

  assign out_0 = test(in_0);

endmodule
