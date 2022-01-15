`begin_keywords "1364-2005"
module top;
  reg passed = 1'b1;
  wire out, cout0, cout1;
  reg sel, in_1, in_0;
  reg pout;

`ifdef __ICARUS__
  // This is technically incorrect for 1'bz inputs. The standard
  // states that we should produce 1'bx for that case (idiotic)!
  localparam zz_blend = 1'bz;
`else
  localparam zz_blend = 1'bx;
`endif

  assign cout0 = sel ? 1'bz : in_0;
  assign cout1 = sel ? in_1: 1'bz;
  assign out = sel ? in_1: in_0;

  task automatic check;
    input bit, in_1, in_0;
    input [63:0] comment;
    begin
      if (sel === 1'b1) begin
        if (bit !== in_1) begin
          $display("FAILED: %0s sel = 1'b1, expected %b, got %b",
                   comment, in_1, bit);
          passed = 1'b0;
        end
      end else if (sel === 1'b0) begin
        if (bit !== in_0) begin
          $display("FAILED: %0s sel = 1'b0, expected %b, got %b",
                   comment, in_0, bit);
          passed = 1'b0;
        end
      end else begin
        if (in_0 === 1'bz && in_1 === 1'bz && bit !== zz_blend) begin
          $display("FAILED: %0s sel = 1'bx/z & ins = %b, expected 1'b%b, got %b",
                   comment, in_0, zz_blend, bit);
          passed = 1'b0;
        end else if (in_0 === in_1 && in_0 !== bit) begin
          $display("FAILED: %0s sel = 1'bx/z & ins = %b, expected 1'b%b, got %b",
                   comment, in_0, in_0, bit);
          passed = 1'b0;
        end else if (in_0 !== in_1 && bit !== 1'bx) begin
          $display("FAILED: %0s sel = 1'bx/z & %b %b, expected 1'bx, got %b",
                   comment, in_1, in_0, bit);
          passed = 1'b0;
        end
      end
    end
  endtask

  // Check the 1 case as a constant Z
  always @(cout0) begin
    check(cout0, 1'bz, in_0, "CZ 1");
  end

  // Check the 0 case as a constant Z
  always @(cout1) begin
    check(cout1, in_1, 1'bz, "CZ 0");
  end

  // Check the continuous assign
  always @(out) begin
    check(out, in_1, in_0, "CA");
  end

  // Check procedural assign.
  always @(sel, in_1, in_0) begin
    check(sel ? in_1 : in_0, in_1, in_0, "PR");
  end

  initial begin
    #1 sel = 1'b1;
    #1 in_1 = 1'b0;
    #1 in_1 = 1'b1;
    #1 in_1 = 1'bz;
    #1 in_1 = 1'bx;

    #1 sel = 1'b0;
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;

    #1 sel = 1'bx;
    #1 in_1 = 1'b0; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;
    #1 in_1 = 1'b1; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;
    #1 in_1 = 1'bz; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;
    #1 in_1 = 1'bx; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;

    #1 sel = 1'bz;
    #1 in_1 = 1'b0; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;
    #1 in_1 = 1'b1; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;
    #1 in_1 = 1'bz; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;
    #1 in_1 = 1'bx; //
    #1 in_0 = 1'b0;
    #1 in_0 = 1'b1;
    #1 in_0 = 1'bz;
    #1 in_0 = 1'bx;

    #1 if (passed) $display("PASSED");
  end
endmodule
`end_keywords
