// Check that an edge cannot be used with a real member of a virtual
// interface.
interface bus_if;
  real r;
endinterface

module test;
  bus_if b0();
  virtual bus_if v;
  initial begin
    v = b0;
    @(posedge v.r);
    $display("FAILED");
  end
endmodule
