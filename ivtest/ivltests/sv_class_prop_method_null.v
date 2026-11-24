// Check comparisons of a method called through a class-handle property.

module test;

  class P;
  endclass

  class Q;
    P p;
    function P get();
      return p;
    endfunction
  endclass

  class Owner;
    Q q;
    function bit has();
      return (q.get() != null);
    endfunction
    function bit is_null();
      return (q.get() == null);
    endfunction
  endclass

  Owner o;
  P direct;
  bit failed = 0;

  initial begin
    o = new;
    o.q = new;
    o.q.p = new;

    if (!o.has()) begin
      $display("FAILED: q.get() != null");
      failed = 1;
    end
    if (o.is_null()) begin
      $display("FAILED: q.get() == null");
      failed = 1;
    end

    direct = o.q.get();
    if (direct == null) begin
      $display("FAILED: direct assignment");
      failed = 1;
    end

    o.q.p = null;
    if (o.has()) begin
      $display("FAILED: null handle != null");
      failed = 1;
    end
    if (!o.is_null()) begin
      $display("FAILED: null handle == null");
      failed = 1;
    end

    if (!failed)
      $display("PASSED");
  end

endmodule
