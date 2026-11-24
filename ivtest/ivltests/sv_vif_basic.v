// Check that a virtual interface variable in a module can be assigned an
// interface instance, and that reads and writes through the variable access
// the instance that the variable currently refers to.

interface bus_if;
  logic [7:0] data;
  logic       valid;
endinterface

module test;

  bus_if a();
  bus_if b();

  virtual bus_if vif;
  virtual interface bus_if vif2;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d). '%s' expected %0h, got %0h", `__LINE__, \
               `"val`", (exp), (val)); \
      failed = 1'b1; \
    end \
  while (0)

  initial begin
    a.data = 8'h11;
    b.data = 8'h22;
    a.valid = 1'b0;
    b.valid = 1'b1;

    `check(vif == null, 1'b1);

    vif = a;
    `check(vif != null, 1'b1);
    `check(vif.data, 8'h11);
    `check(vif.valid, 1'b0);

    vif = b;
    `check(vif.data, 8'h22);
    `check(vif.valid, 1'b1);

    // Blocking writes go to the selected instance only
    vif.data = 8'h33;
    `check(b.data, 8'h33);
    `check(a.data, 8'h11);

    // Copy of a handle
    vif2 = vif;
    vif = a;
    `check(vif2 == vif, 1'b0);
    `check(vif2.data, 8'h33);
    `check(vif.data, 8'h11);
    vif.data = vif2.data + 8'h01;
    `check(a.data, 8'h34);
    `check(b.data, 8'h33);

    // Non-blocking writes
    vif.data <= 8'h55;
    vif2.valid <= 1'b0;
    `check(a.data, 8'h34);
    `check(b.valid, 1'b1);
    #1;
    `check(a.data, 8'h55);
    `check(b.data, 8'h33);
    `check(b.valid, 1'b0);
    `check(a.valid, 1'b0);

    vif2 = a;
    `check(vif2 == vif, 1'b1);
    vif = null;
    `check(vif == null, 1'b1);
    `check(vif2 != null, 1'b1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
