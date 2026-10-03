// Check zero padding for out-of-bounds string selects in constant functions.

module test;

  function [7:0] f(input string s, input integer index);
    f = s[index];
  endfunction

  // Keep the indices out of bounds for both bit and character indexing.
  localparam [7:0] HIGH = f("abc", 32);
  localparam [7:0] LOW = f("abc", -32);
  localparam [7:0] EMPTY = f("", 32);

  initial begin
    if (HIGH !== 8'h00 || LOW !== 8'h00 || EMPTY !== 8'h00) begin
      $display("FAILED: out-of-bounds string select did not return zero");
    end else begin
      $display("PASSED");
    end
  end

endmodule
