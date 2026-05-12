// Check that edge event controls can not be used with named events.

module test;

  event e;

  initial begin
    @(edge e);
  end

endmodule
