// Check two-state conversion of packed member targets in constant functions.

module test;

  function [11:0] convert(input [7:0] value, input selected);
    struct packed {
      logic [3:0] guard;
      bit [3:0] two;
      logic [3:0] four;
    } data;

    data = 12'ha5f;
    if (selected) begin
      {data.two[1:0], {data.four[1:0]}} = value[3:0];
    end else begin
      {data.two, data.four} = value;
    end
    convert = data;
  endfunction

  localparam [11:0] WHOLE = convert(8'bx1z0_z01x, 0);
  localparam [11:0] SELECTED = convert(8'b0000_xzzx, 1);

  bit failed = 1'b0;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(WHOLE, 12'b1010_0100_z01x);
    `check(SELECTED, 12'b1010_0100_11zx);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
