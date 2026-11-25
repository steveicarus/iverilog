// Check that a virtual interface of a different interface cannot be assigned
// to a virtual interface.

interface bus_if;
  logic [7:0] data;
endinterface

interface other_if;
  logic [7:0] data;
endinterface

module test;

  bus_if a();
  other_if b();

  virtual bus_if vif;
  virtual other_if vif2;

  initial begin
    vif2 = b;
    vif = vif2;
    $display("FAILED");
  end

endmodule
