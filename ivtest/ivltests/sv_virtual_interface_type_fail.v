// Check that assigning an instance of another interface to a virtual interface
// is an error.

interface bus_if;
  logic [7:0] data = 0;
endinterface

interface other_if;
  logic [7:0] data = 0;
endinterface

module test;
  bus_if b0();
  other_if o0();
  virtual bus_if v;

  initial begin
    v = o0;
    $display("FAILED");
  end
endmodule
