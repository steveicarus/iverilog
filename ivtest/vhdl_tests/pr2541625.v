module top();
  wire out1, out2;

  child c1(1, 0, out1);
  child c2(1, 1, out2);

  initial begin
    #1;
    if (out1 !== 0)
      $display("FAILED -- out1 !== 0");
    else if (out2 !== 1)
      $display("FAILED -- out2 !== 1");
    else
      $display("PASSED");
  end

endmodule // top

module child(in1, in2, out);
  input in1, in2;
  output out;

  assign out = in1 & in2;
endmodule // child
