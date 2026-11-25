// Check that an access to a name that is not in the interface is an error.

interface bus_if;
  logic [7:0] data;
endinterface

module test;

  bus_if a();

  virtual bus_if vif;

  initial begin
    vif = a;
    vif.nothing = 8'h00;
    $display("FAILED");
  end

endmodule
