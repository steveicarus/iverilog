// Check calls of an overridden virtual void function as a statement:
// through a handle of the base class, of the derived class, with an
// implicit this and with this, and over three levels of classes.

module test;

  class A;
    int log;
    virtual function void pv(int v = 1);
      log = log * 10 + 1;
    endfunction
    function void bare();
      pv();
    endfunction
  endclass

  class B extends A;
    function void pv(int v = 2);
      log = log * 10 + 2;
    endfunction
    function void bare_b();
      pv();
      this.pv();
    endfunction
  endclass

  class C extends B;
    function void pv(int v = 3);
      log = log * 10 + v;
    endfunction
  endclass

  A a;
  B b;
  C c;
  int errors = 0;

  initial begin
    b = new;
    b.pv();
    a = b;
    a.pv();
    if (b.log != 22) errors += 1;

    b.log = 0;
    b.bare();
    b.bare_b();
    if (b.log != 222) errors += 1;

    c = new;
    c.pv();
    c.pv(7);
    // Default values come from the class of the handle
    b = c;
    b.pv();
    a = c;
    a.pv(5);
    c.bare_b();
    if (c.log != 372522) begin
      $display("log=%0d", c.log);
      errors += 1;
    end

    if (errors == 0)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
