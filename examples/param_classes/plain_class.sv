// Regression: plain (non-parameterized) class still works.
class counter;
  int n;
  function new(); n = 0; endfunction
  function void inc(); n++; endfunction
endclass

module top;
  counter c;
  initial begin
    c = new;
    c.inc();
    c.inc();
    if (c.n !== 2) begin
      $display("FAIL n=%0d", c.n);
      $finish(1);
    end
    $display("PASS plain_class n=%0d", c.n);
    $finish;
  end
endmodule
