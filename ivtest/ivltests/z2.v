module baz;
  parameter Q = 10;
endmodule

module bar;
  parameter P = 1;
  baz #(P+1) baz1();
endmodule

module foo;
  bar #3 bar1();
  initial
    $display("PASSED");
endmodule
