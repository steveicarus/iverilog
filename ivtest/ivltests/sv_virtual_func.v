// Check that a call of a virtual function through a handle of the base
// class runs the override of the class of the object. IEEE 1800-2017 8.20.

module test;

  class Base;
    int cnt;
    virtual function string who();
      return "Base";
    endfunction
    function string plain();
      return "Base";
    endfunction
    // A call of a virtual function on the implicit this is dispatched too.
    function string call_who();
      return who();
    endfunction
    virtual function int bump();
      cnt += 1;
      return cnt;
    endfunction
  endclass

  class Mid extends Base;
    // virtual because the function in Base is
    function string who();
      return "Mid";
    endfunction
    function string plain();
      return "Mid";
    endfunction
  endclass

  class Leaf extends Mid;
    virtual function string who();
      return "Leaf";
    endfunction
    function int bump();
      cnt += 10;
      return cnt;
    endfunction
  endclass

  // Does not override anything
  class Other extends Base;
  endclass

  // The override calls the function that it overrides
  class Chain extends Leaf;
    virtual function string who();
      return {"Chain>", super.who()};
    endfunction
  endclass

  Base b;
  Mid m;
  Leaf l;
  Other o;
  Chain c;
  Base h;
  Base arr[5];
  int errors = 0;

  task check(string got, string exp);
    if (got != exp) begin
      $display("FAILED: got %0s, expected %0s", got, exp);
      errors += 1;
    end
  endtask

  initial begin
    b = new;
    check(b.who(), "Base");

    m = new;
    b = m;
    check(b.who(), "Mid");
    // Not virtual: the class of the handle decides
    check(b.plain(), "Base");
    check(m.plain(), "Mid");
    check(m.who(), "Mid");

    l = new;
    b = l;
    m = l;
    check(b.who(), "Leaf");
    check(m.who(), "Leaf");
    check(l.who(), "Leaf");
    check(b.call_who(), "Leaf");

    // Inherited without override
    o = new;
    b = o;
    check(b.who(), "Base");

    m = new;
    l = new;
    c = new;
    arr[0] = new;
    arr[1] = m;
    arr[2] = l;
    arr[3] = o;
    arr[4] = c;
    for (int idx = 0 ; idx < 5 ; idx += 1) begin
      string exp;
      // Methods are called through a handle, not an array element
      h = arr[idx];
      case (idx)
	0, 3: exp = "Base";
	1: exp = "Mid";
	2: exp = "Leaf";
	4: exp = "Chain>Leaf";
      endcase
      check(h.who(), exp);
      check(h.call_who(), exp);
    end

    // State kept in the object
    b = l;
    if (b.bump() != 10 || b.bump() != 20) begin
      $display("FAILED: bump");
      errors += 1;
    end

    if (errors == 0)
      $display("PASSED");
  end

endmodule
