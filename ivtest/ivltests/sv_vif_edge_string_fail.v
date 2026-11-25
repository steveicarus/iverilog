// Check that an edge cannot be used with a string member of a virtual
// interface.
interface bus_if;
  string s;
endinterface

module test;
  bus_if b0();
  virtual bus_if v;
  initial begin
    v = b0;
    @(negedge v.s);
    $display("FAILED");
  end
endmodule
