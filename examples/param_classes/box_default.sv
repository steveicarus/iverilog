// Smoke: ANSI class parameter port list with defaults (no explicit #() override).
class box #(type T = int, parameter int W = 8);
  T val;
  bit [W-1:0] data;
  function new(T v);
    val = v;
    data = v;
  endfunction
  function T get();
    return val;
  endfunction
endclass

module top;
  box b;
  initial begin
    b = new(42);
    if (b.get() !== 42) begin
      $display("FAIL: get=%0d", b.get());
      $finish(1);
    end
    if (b.data !== 8'h2a) begin
      $display("FAIL: data=%0h", b.data);
      $finish(1);
    end
    $display("PASS box_default val=%0d data=%0h", b.val, b.data);
    $finish;
  end
endmodule
