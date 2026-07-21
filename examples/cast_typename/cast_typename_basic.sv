// Tier A #7 smoke: $cast (class handles) + $typename (static type string).
class base; endclass
class derived extends base;
  int x;
  function new(); x = 7; endfunction
endclass

module top;
  base b, b2;
  derived d, d2;
  string tn;
  bit ok;
  int pass;

  initial begin
    pass = 1;

    d = new;
    b = d;

    ok = $cast(d2, b);
    if (!ok || d2 == null || d2.x !== 7) begin
      $display("FAIL: downcast ok=%0b d2.x=%0d", ok, d2 == null ? -1 : d2.x);
      pass = 0;
    end

    // Upcast always succeeds for a live derived object.
    ok = $cast(b2, d);
    if (!ok || b2 == null) begin
      $display("FAIL: upcast");
      pass = 0;
    end

    // Task form: same dynamic check / assign.
    d2 = null;
    $cast(d2, b);
    if (d2 == null || d2.x !== 7) begin
      $display("FAIL: task-form $cast");
      pass = 0;
    end

    // Incompatible: base-only object into derived.
    b2 = new;
    ok = $cast(d2, b2);
    if (ok) begin
      $display("FAIL: expected incompatible cast to fail");
      pass = 0;
    end

    tn = $typename(d);
    if (tn != "class derived") begin
      $display("FAIL: $typename(d)=\"%s\" want \"class derived\"", tn);
      pass = 0;
    end

    tn = $typename(b);
    if (tn != "class base") begin
      $display("FAIL: $typename(b)=\"%s\" want \"class base\"", tn);
      pass = 0;
    end

    $display("typename=%s", $typename(d));
    if (pass) $display("PASSED");
    else $fatal(1, "cast/typename smoke failed");
  end
endmodule
