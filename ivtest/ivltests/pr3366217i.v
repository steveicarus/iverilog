/*
 * Check that the initial value can be out of range and that the next()/prev()
 * enumeration methods do not change to a defined state.
 */
module top;
  reg pass;
  enum bit [3:0] {a2 = 1, b2 = 2, c2 = 3, d2 = 4} evar2;
  enum reg [3:0] {a4 = 1, b4 = 2, c4 = 3, d4 = 4} evar4;

  initial begin
    pass = 1'b1;

    if (evar2 !== 0) begin
      $display("Failed initial/2 value should be 0, got %d", evar2);
      pass = 1'b0;
    end
    if (evar4 !== 4'bx) begin
      $display("Failed initial/4 value should be 'bx, got %d", evar4);
      pass = 1'b0;
    end

    evar2 = evar2.next;
    if (evar2 !== 0) begin
      $display("Failed next/2 of an invalid value should be 0, got %d", evar2);
      pass = 1'b0;
    end
    evar4 = evar4.next;
    if (evar4 !== 4'bx) begin
      $display("Failed next/4 of an invalid value should be 0, got %d", evar4);
      pass = 1'b0;
    end

    evar2 = evar2.prev;
    if (evar2 !== 0) begin
      $display("Failed prev/2 of an invalid value should be 0, got %d", evar2);
      pass = 1'b0;
    end
    evar4 = evar4.prev;
    if (evar4 !== 4'bx) begin
      $display("Failed prev/4 of an invalid value should be 0, got %d", evar4);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
