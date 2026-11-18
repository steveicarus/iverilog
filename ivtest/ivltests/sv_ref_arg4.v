// A property of a class object can be the actual of a ref argument.

class box;
  int n;
  real r;
  string s;
  logic [7:0] bits;
endclass

class holder;
  box b;
  int count;

  function new();
    b = new;
    count = 3;
  endfunction

  task bump(ref int x);
    x = x + 10;
  endtask

  // The property is named without an object here.
  task bump_count();
    bump(count);
  endtask
endclass

module top;
  task automatic set_n(ref int x, input int v);
    #1;
    x = v;
  endtask

  task automatic set_r(ref real x);
    x = x * 2.0;
  endtask

  task automatic set_s(ref string x);
    x = {x, "!"};
  endtask

  task automatic set_bits(ref logic [7:0] x);
    x[3:0] = 4'hf;
  endtask

  initial begin
    box bx;
    holder h;
    bx = new;
    bx.n = 1;
    bx.r = 1.5;
    bx.s = "hi";
    bx.bits = 8'h50;

    set_n(bx.n, 42);
    set_r(bx.r);
    set_s(bx.s);
    set_bits(bx.bits);

    if (bx.n !== 42) begin
      $display("FAILED: n=%0d", bx.n);
      $finish;
    end
    if (bx.r != 3.0) begin
      $display("FAILED: r=%f", bx.r);
      $finish;
    end
    if (bx.s != "hi!") begin
      $display("FAILED: s=%s", bx.s);
      $finish;
    end
    if (bx.bits !== 8'h5f) begin
      $display("FAILED: bits=%h", bx.bits);
      $finish;
    end

    h = new;
    h.bump(h.count);
    if (h.count !== 13) begin
      $display("FAILED: count=%0d (expect 13)", h.count);
      $finish;
    end
    h.bump_count();
    if (h.count !== 23) begin
      $display("FAILED: count=%0d (expect 23)", h.count);
      $finish;
    end

    $display("PASSED");
  end
endmodule
