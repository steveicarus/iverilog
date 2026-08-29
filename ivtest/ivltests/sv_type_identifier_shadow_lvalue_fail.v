// Check that a typedef prevents an l-value from binding an outer variable.

module test;

  integer value;

  task set_value(output integer result);
    result = 1;
  endtask

  initial begin : inner
    typedef logic value;
    set_value(value);
  end

endmodule
