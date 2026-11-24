// Check that a virtual interface can refer to an interface port, to an
// element of an array of instances and to an instance in another module,
// and that members that are only accessed through a virtual interface are
// kept in every instance.

interface bus_if;
  logic [7:0] data;
  logic [7:0] only_vif = 8'h00;
  wire  [7:0] undriven;
  int         id = 0;
endinterface

module sub(bus_if p, input int id);

  virtual bus_if v;

  initial begin
    v = p;
    v.id = id;
    v.only_vif <= id + 8'h40;
  end

  task automatic put(logic [7:0] val);
    v.data = val;
  endtask

endmodule

module test;

  bus_if a();
  bus_if arr[2]();

  sub s0(a, 1);
  sub s1(arr[0], 2);
  sub s2(arr[1], 3);

  virtual bus_if vif;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", (exp), (val)); \
      failed = 1'b1; \
    end \
  while (0)

  initial begin
    #1;
    vif = a;
    `check(vif.id, 1);
    `check(vif.only_vif, 8'h41);
    `check(vif.undriven, 8'hzz);

    vif = arr[0];
    `check(vif.id, 2);
    `check(vif.only_vif, 8'h42);

    vif = arr[1];
    `check(vif.id, 3);
    `check(vif.only_vif, 8'h43);
    vif.only_vif = 8'h53;

    vif = s1.p;
    `check(vif.id, 2);
    `check(vif.only_vif, 8'h42);
    `check(vif == s1.v, 1'b1);
    `check(vif != s2.v, 1'b1);

    s0.put(8'h10);
    s1.put(8'h11);
    s2.put(8'h12);
    `check(a.data, 8'h10);
    `check(vif.data, 8'h11);
    vif = s2.v;
    `check(vif.data, 8'h12);
    `check(vif.only_vif, 8'h53);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
