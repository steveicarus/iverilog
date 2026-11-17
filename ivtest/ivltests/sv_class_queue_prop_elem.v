// Regression: element reads and writes, including the [$] last element,
// of string, real and int queue and dynamic array class properties,
// both inside methods and through a class handle.

module test;

  class C;
    string s[$];
    real r[$];
    int i[$];
    string d[];
    real e[];
    int f[];

    function void setup();
      s.push_back("a"); s.push_back("b");
      r.push_back(1.5); r.push_back(2.5);
      i.push_back(7); i.push_back(8);
      d = new[2]; e = new[2]; f = new[2];
    endfunction

    function void write();
      s[1] = "z"; r[0] = 9.5; i[0] = 3;
      d[1] = "q"; e[1] = 4.5; f[1] = 6;
    endfunction

    function void write_last();
      s[$] = "last"; r[$] = 7.25; i[$] = 99;
    endfunction

    function string get_s(int n); return s[n]; endfunction
    function real get_r(int n); return r[n]; endfunction
    function string get_last(); return s[$]; endfunction
  endclass

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). expected %0d, got %0d", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  `define checks(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). expected '%s', got '%s'", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  `define checkr(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). expected %f, got %f", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  C c;

  initial begin
    c = new;
    c.setup();

    // Reads
    `checks(c.s[0], "a");
    `checks(c.s[1], "b");
    `checkr(c.r[1], 2.5);
    `check(c.i[1], 8);
    `checks(c.d[0], "");
    `checkr(c.e[0], 0.0);
    `checks(c.s[5], "");
    `checkr(c.r[5], 0.0);
    `checks(c.get_s(1), "b");
    `checkr(c.get_r(0), 1.5);
    `checks(c.s[$], "b");
    `checkr(c.r[$], 2.5);
    `check(c.i[$], 8);
    `checks(c.get_last(), "b");

    // Writes inside a method
    c.write();
    `checks(c.s[0], "a");
    `checks(c.s[1], "z");
    `checkr(c.r[0], 9.5);
    `check(c.i[0], 3);
    `checks(c.d[1], "q");
    `checkr(c.e[1], 4.5);
    `check(c.f[1], 6);
    `check(c.f[0], 0);

    // Writes to the last element
    c.write_last();
    `checks(c.s[1], "last");
    `checkr(c.r[1], 7.25);
    `check(c.i[1], 99);

    // Writes through the handle
    c.s[0] = "ext";
    c.r[0] = 1.25;
    c.d[0] = "dd";
    c.s[$] = "ext_last";
    `checks(c.s[0], "ext");
    `checks(c.s[1], "ext_last");
    `checkr(c.r[0], 1.25);
    `checks(c.d[0], "dd");
    `check(c.s.size(), 2);

    // Empty queue: [$] is out of range and reads the default
    c = new;
    `checks(c.s[$], "");
    `checkr(c.r[$], 0.0);

    if (!failed)
      $display("PASSED");
  end

endmodule
