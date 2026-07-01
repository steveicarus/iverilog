// Check that posedge event controls can not be used with named events.

module test;

  event e;

  initial begin
    @(posedge e);
  end

endmodule
