// Check that negedge event controls can not be used with named events.

module test;

  event e;

  initial begin
    @(negedge e);
  end

endmodule
