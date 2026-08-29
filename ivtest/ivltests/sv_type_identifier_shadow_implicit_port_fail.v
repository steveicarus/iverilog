// Check that a typedef prevents an implicit named port connection from
// binding an outer variable.

module M(input integer value);
endmodule

module test;

  integer value;

  if (1) begin : inner
    typedef logic value;
    M i_m(.value);
  end

endmodule
