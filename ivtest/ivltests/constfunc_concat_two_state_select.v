// Check two-state conversion of array words and part selects in constant functions.

module test;

  function [19:0] convert(input [15:0] value);
    bit [7:0] two[0:1];
    logic [3:0] four;

    two[1] = 8'ha5;
    {two[0], two[1][3:0], four} = value;
    convert = {two[0], two[1], four};
  endfunction

  localparam [19:0] VALUE = convert(16'bx1z0_10xz_z1x0_xz01);

  initial begin
    if (VALUE === 20'b01001000_10100100_xz01) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected 01001000_10100100_xz01, got %b", VALUE);
    end
  end

endmodule
