// a string member read and written through a handle
interface bus_if;
  string name;
endinterface
module test;
  bus_if b1();
  bus_if b2();
  virtual bus_if v;
  string s;
  bit pass = 1;
  initial begin
    b1.name = "one"; b2.name = "two";
    v = b2;
    s = v.name;
    if (s != "two" || v.name != "two") begin $display("FAILED: b2 %s", s); pass = 0; end
    v = b1;
    s = v.name;
    if (s != "one" || v.name != "one") begin $display("FAILED: b1 %s", s); pass = 0; end
    v.name = "uno";
    if (b1.name != "uno" || b2.name != "two") begin $display("FAILED: write %s %s", b1.name, b2.name); pass = 0; end
    if (pass) $display("PASSED");
  end
endmodule
