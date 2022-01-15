module bar;
  parameter P = 1;
endmodule

module baz;
  parameter P = 1;
  parameter Q = 2;
endmodule

module foo;
  bar #345 bar1();
  bar #(456) bar2();
  baz baz1();
  baz #(888,999) baz2();

  initial
    $display("PASSED");
endmodule
