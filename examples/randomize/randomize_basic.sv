// Unconstrained class.randomize() / rand vertical slice smoke test.
class pkt;
  rand bit [7:0] a;
  rand bit [3:0] b;
  bit [7:0] c;
  function new();
    c = 8'h11;
  endfunction
endclass

module top;
  pkt p;
  bit ok;
  initial begin
    p = new;
    ok = p.randomize();
    if (!ok) $fatal(1, "randomize failed");
    // a,b should be randomized (not X); c unchanged
    if (p.a === 8'hxx) $fatal(1, "rand a still X");
    if (p.b === 4'hx)  $fatal(1, "rand b still X");
    if (p.c !== 8'h11) $fatal(1, "non-rand mutated");
    $display("a=%0h b=%0h c=%0h", p.a, p.b, p.c);
    $display("PASSED");
  end
endmodule
