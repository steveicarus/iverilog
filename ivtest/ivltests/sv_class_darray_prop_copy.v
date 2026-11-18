// Check that assigning a dynamic array to or from a class property copies
// the array. A dynamic array is a value, only the class handle is a
// reference.

module test;

  typedef int da_t[];

  class C;
    int d[];
    real rd[];
    string sd[];
    function da_t get();
      return d;
    endfunction
  endclass

  bit failed = 1'b0;
  task check(bit ok, string what);
    if (!ok) begin
      $display("FAILED: %s", what);
      failed = 1'b1;
    end
  endtask

  function void clobber(int a[]);
    a[0] = 99;
  endfunction

  task clobber_task(int a[]);
    a[0] = 99;
  endtask

  C c, c2;
  int v[], v2[];
  real rv[];
  string sv[];

  initial begin
    c = new;
    c2 = new;

    v = '{1, 2, 3};
    c.d = v;
    v[0] = 7;
    check(c.d[0] == 1, "property = variable, change variable");
    c.d.reverse();
    check(v[0] == 7, "property = variable, change property");

    v = '{1, 2, 3};
    c.d = v;
    v2 = c.d;
    v2[0] = 7;
    check(c.d[0] == 1, "variable = property, change variable");
    c.d.reverse();
    check(v2[0] == 7, "variable = property, change property");

    c.d = '{1, 2, 3};
    c2.d = c.d;
    c2.d.reverse();
    check(c.d[0] == 1, "property = property, other object");

    c.d = '{1, 2, 3};
    v2 = c.get();
    v2[0] = 7;
    check(c.d[0] == 1, "return of a property, change variable");
    c.d.reverse();
    check(v2[0] == 7, "return of a property, change property");

    c.d = '{1, 2, 3};
    clobber(c.d);
    check(c.d[0] == 1, "property as function argument");
    clobber_task(c.d);
    check(c.d[0] == 1, "property as task argument");

    v = '{1, 2, 3};
    c.d = v;
    c.d[0] = 9;
    check(v[0] == 1, "element write to property, variable unchanged");
    v[1] = 8;
    check(c.d[1] == 2, "element write to variable, property unchanged");

    c.d = new[4];
    check(c.d.size() == 4, "property = new[]");

    rv = '{1.5, 2.5};
    c.rd = rv;
    rv[0] = 9.5;
    check(c.rd[0] == 1.5, "real array, change variable");
    c.rd[1] = 8.5;
    check(rv[1] == 2.5, "real array, change property");

    sv = '{"a", "b"};
    c.sd = sv;
    sv[0] = "z";
    check(c.sd[0] == "a", "string array, change variable");
    c.sd[1] = "y";
    check(sv[1] == "b", "string array, change property");

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
