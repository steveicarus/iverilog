primitive passthrough (o, i);
  input i;
  output o;
  table
  // i : o
     1 : 1;
     0 : 0;
     ? : 0;
  endtable
endprimitive

module test;
  reg i;
  wire o1, o1b, o2, o2b;

  initial begin
    i = 1'b0;
    #1;
    if ((o1 !== 1'b0) && (o1b !== 1'b1) &&
        (o2 !== 1'b0) && (o2b !== 1'b1)) $display("FAILED");
    else $display("PASSED");
  end

  passthrough (o1, i);
  passthrough (o1b, !i);
  passthrough (o2, i);
  passthrough (o2b, ~i);
endmodule
