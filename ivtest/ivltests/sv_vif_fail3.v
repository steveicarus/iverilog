// Check that an instance of a different interface cannot be assigned to a
// virtual interface.

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

  initial begin
    vif = b;
    $display("FAILED");
  end

endmodule
