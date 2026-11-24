// Check that a member of a virtual interface is rejected in a continuous
// assignment (IEEE1800-2017 25.9).

interface bus_if;
  logic [7:0] data;
endinterface

module test;

  bus_if a();

  virtual bus_if vif = a;
  wire [7:0] w;

  assign w = vif.data;

  initial begin
    $display("FAILED");
  end

endmodule
