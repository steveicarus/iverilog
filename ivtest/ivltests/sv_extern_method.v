// Check methods declared extern in a class and defined outside of it.

module test;
  int errors = 0;

  class Base;
    int val;
    string name;
    extern function new(int v);
    extern function int get();
    extern function void set(int v);
    extern function int add(int a, int b = 1);
    extern task bump(int n);
    extern virtual function string who();
    extern virtual task hello();
    extern function logic [3:0] nibble();
    extern static function int answer();
  endclass

  class Derived extends Base;
    extern function new(int v);
    extern virtual function string who();
    extern virtual task hello();
  endclass

  function Base::new(int v);
    val = v;
    name = "Base";
  endfunction : new

  function int Base::get();
    return this.val;
  endfunction : get

  function void Base::set(int v);
    val = v;
  endfunction

  function int Base::add(int a, int b = 1);
    return val + a + b;
  endfunction

  task Base::bump(int n);
    val += n;
  endtask : bump

  function string Base::who();
    return "Base";
  endfunction

  task Base::hello();
    $display("hello from %s", who());
  endtask

  function logic [3:0] Base::nibble();
    return val & 4'hf;
  endfunction

  function int Base::answer();
    return 42;
  endfunction

  function Derived::new(int v);
    super.new(v + 100);
    name = "Derived";
  endfunction

  function string Derived::who();
    return {"Derived/", super.who()};
  endfunction

  task Derived::hello();
    $display("hello from derived, val=%0d", val);
  endtask

  Base b;
  Derived d;

  initial begin
    b = new(5);
    if (b.get() !== 5) begin
      $display("FAILED: get=%0d", b.get());
      errors += 1;
    end
    b.set(7);
    b.bump(3);
    if (b.get() !== 10) begin
      $display("FAILED: after set/bump get=%0d", b.get());
      errors += 1;
    end
    if (b.add(1) !== 12 || b.add(1, 2) !== 13) begin
      $display("FAILED: add");
      errors += 1;
    end
    if (b.nibble() !== 4'ha) begin
      $display("FAILED: nibble=%h", b.nibble());
      errors += 1;
    end
    if (b.who() != "Base") begin
      $display("FAILED: who=%s", b.who());
      errors += 1;
    end
    b.hello();

    d = new(1);
    if (d.get() !== 101 || d.name != "Derived") begin
      $display("FAILED: derived get=%0d name=%s", d.get(), d.name);
      errors += 1;
    end

    // Calls through a base class handle reach the derived definitions.
    b = d;
    if (b.who() != "Derived/Base") begin
      $display("FAILED: virtual who=%s", b.who());
      errors += 1;
    end
    b.hello();

    if (errors == 0)
      $display("PASSED");
  end
endmodule
