// Check that === and !== cannot be used with a virtual interface
// (IEEE1800-2017 25.9 lists only == and !=).
interface bus_if;
  logic [7:0] data;
endinterface

module test;
  bus_if b0();
  virtual bus_if v;
  initial begin
    v = b0;
    if (v === b0) $display("FAILED");
    if (v !== null) $display("FAILED");
  end
endmodule
