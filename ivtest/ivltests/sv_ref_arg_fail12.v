// Check that a parameter, localparam or specparam cannot be passed to a
// const ref argument. Only variables can be passed by reference
// (IEEE 1800-2017 13.5.2).

module test;
  parameter P = 5;
  localparam L = 6;
  specify
    specparam S = 7;
  endspecify

  task automatic t(const ref int x);
    $display("%0d", x);
  endtask

  initial begin
    t(P);
    t(L);
    t(S);
  end
endmodule
