// Check that a virtual interface can be a class property and an argument of
// a constructor, a task and a function, and that every handle keeps the
// instance it was given when other handles change.

interface bus_if;
  logic [7:0] data;
  logic [7:0] resp;
endinterface

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", (exp), (val)); \
      failed = 1'b1; \
    end \
  while (0)

  class driver;
    virtual bus_if vif;
    virtual interface bus_if other;

    function new(virtual bus_if v);
      vif = v;
    endfunction

    function logic [7:0] get();
      return vif.data;
    endfunction

    task put(logic [7:0] val);
      vif.data = val;
      this.vif.resp <= ~val;
    endtask

    function void set_other(virtual interface bus_if v);
      other = v;
    endfunction

    function virtual bus_if get_vif();
      return vif;
    endfunction
  endclass

  bus_if a();
  bus_if b();

  virtual bus_if vif;
  virtual bus_if tmp;
  driver da, db;

  task automatic write(virtual bus_if v, input logic [7:0] val);
    v.data = val;
  endtask

  function automatic logic [7:0] read(virtual bus_if v);
    return v.data;
  endfunction

  initial begin
    vif = a;
    da = new(vif);
    vif = b;
    db = new(vif);

    a.data = 8'h01;
    b.data = 8'h02;

    // The objects keep the instance they were made with
    `check(da.get(), 8'h01);
    `check(db.get(), 8'h02);
    `check(da.vif.data, 8'h01);
    `check(db.vif.data, 8'h02);

    vif = null;
    `check(da.get(), 8'h01);
    `check(da.vif != null, 1'b1);
    `check(da.other == null, 1'b1);

    da.put(8'h10);
    db.put(8'h20);
    `check(a.data, 8'h10);
    `check(b.data, 8'h20);
    #1;
    `check(a.resp, 8'hef);
    `check(b.resp, 8'hdf);

    // Access to a property from outside of the class
    da.vif.data = 8'h11;
    db.vif.data <= 8'h21;
    #1;
    `check(a.data, 8'h11);
    `check(b.data, 8'h21);

    // Copy between objects
    da.set_other(db.vif);
    `check(da.other == db.vif, 1'b1);
    `check(da.other.data, 8'h21);
    da.other = da.vif;
    `check(da.other.data, 8'h11);
    `check(db.vif.data, 8'h21);

    // Task and function arguments
    write(a, 8'h31);
    write(db.vif, 8'h32);
    `check(a.data, 8'h31);
    `check(b.data, 8'h32);
    `check(read(b), 8'h32);
    `check(read(da.vif), 8'h31);

    // Function result
    tmp = db.get_vif();
    `check(tmp.data, 8'h32);
    tmp = da.get_vif();
    `check(tmp.data, 8'h31);
    `check(tmp == da.vif, 1'b1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
