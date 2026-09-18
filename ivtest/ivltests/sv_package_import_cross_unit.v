// Check that identifiers imported from another compilation unit are visible
// and do not depend on lexical position.

module test;

  import p::parameter_value;
  import p::variable_value;
  import p::event_value;
  import p::*;

  localparam integer imported_parameter = parameter_value;
  localparam integer imported_localparam = localparam_value;
  localparam integer imported_enum = enum_value;
  integer imported_variable;
  integer imported_array;
  reg event_seen;
  reg failed;

  always @(event_value)
    event_seen = 1'b1;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    event_seen = 1'b0;

    #1;
    imported_variable = variable_value;
    array_value[1] = 53;
    imported_array = array_value[1];
    -> event_value;
    #1;

    `check(imported_parameter, 17);
    `check(imported_localparam, 23);
    `check(imported_enum, 31);
    `check(imported_variable, 32'd42);
    `check(imported_array, 32'd53);
    `check(event_seen, 1'b1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
