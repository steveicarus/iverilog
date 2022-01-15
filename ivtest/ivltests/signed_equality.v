module top;
  parameter seeq1 = 6'sb111000 === 4'sb1000;
  parameter seeqx = 6'sbxxx000 === 4'sbx000;
  parameter seeqz = 6'sbzzz000 === 4'sbz000;
  parameter seq1 = 6'sb111000 == 4'sb1000;
  parameter seqx = 6'sbxxx000 == 4'sbx000;
  parameter seqz = 6'sbzzz000 == 4'sbz000;
  reg pass;

  initial begin
    pass = 1'b1;

    if (seeq1 !== 1'b1) begin
      $display("FAILED: signed === (1), got %b", seeq1);
      pass = 1'b0;
    end

    if (seeqx !== 1'b1) begin
      $display("FAILED: signed === (x), got %b", seeqx);
      pass = 1'b0;
    end

    if (seeqz !== 1'b1) begin
      $display("FAILED: signed === (z), got %b", seeqz);
      pass = 1'b0;
    end

    if (seq1 !== 1'b1) begin
      $display("FAILED: signed == (1), got %b", seq1);
      pass = 1'b0;
    end

    if (seqx !== 1'bx) begin
      $display("FAILED: signed == (x), got %b", seqx);
      pass = 1'b0;
    end

    if (seqz !== 1'bx) begin
      $display("FAILED: signed == (z), got %b", seqz);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
