// Multi-dimension unpacked array parameters (GitHub #1180). The value is a
// nested assignment pattern flattened row-major; an index select that consumes
// every unpacked dimension yields the packed element. Exercises constant and
// variable indices, ascending and descending declared ranges.
module test();

  // Ascending 2-D.
  localparam logic [3:0] TBL [0:1][0:3] = '{ '{4'd1, 4'd2, 4'd3, 4'd4},
                                             '{4'd5, 4'd6, 4'd7, 4'd8} };
  // Descending outer dimension, ascending inner.
  localparam logic [3:0] DTBL [1:0][0:3] = '{ '{4'd1, 4'd2, 4'd3, 4'd4},
                                              '{4'd5, 4'd6, 4'd7, 4'd8} };
  // 3-D.
  localparam logic [3:0] CUBE [0:1][0:1][0:1] = '{ '{'{4'd1,4'd2}, '{4'd3,4'd4}},
                                                   '{'{4'd5,4'd6}, '{4'd7,4'd8}} };

  reg [1:0] r, c;
  integer errs = 0;

  task check(input [31:0] got, input [31:0] exp, input [127:0] label);
    if (got !== exp) begin
      errs = errs + 1;
      $display("FAILED: %0s got %0d expected %0d", label, got, exp);
    end
  endtask

  initial begin
    // Constant indices (folded at elaboration).
    check(TBL[0][0], 1, "TBL[0][0]");
    check(TBL[1][3], 8, "TBL[1][3]");
    // For a descending [1:0] range, '{a,b} assigns a->index1, b->index0.
    check(DTBL[0][0], 5, "DTBL[0][0]");
    check(DTBL[1][3], 4, "DTBL[1][3]");
    check(CUBE[1][0][1], 6, "CUBE[1][0][1]");

    // Variable indices (runtime part select of the flat value).
    r = 1; c = 2; #1; check(TBL[r][c], 7, "TBL[r=1][c=2]");
    r = 0; c = 1; #1; check(DTBL[r][c], 6, "DTBL[r=0][c=1]");

    if (errs == 0) $display("PASSED");
    else           $display("FAILED");
  end

endmodule
