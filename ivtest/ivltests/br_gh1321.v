// Test for GitHub issue #1321 - merged fork+upstream coverage
// SystemVerilog allows a statement label written as a prefix before a
// begin-end or fork-join block (IEEE 1800-2017 9.3.5 and Annex A.6.4):
//
//     my_blk : begin ... end  is equivalent to  begin : my_blk ... end

module test;
  reg failed = 1'b0;
  reg done   = 1'b0;
  logic a;

  // Upstream regression: prefix label in always_comb
  always_comb begin
    my_blk: begin
      a = 1'b1;
    end
  end

  initial begin

    // Prefix label on a begin/end block.
    my_seq: begin
      $display("PASS my_seq");
    end

    // Prefix label on a fork/join block.
    my_fork: fork
      $display("PASS my_fork.a");
      $display("PASS my_fork.b");
    join

    // Prefix label with a matching end label is legal.
    my_match: (* ivl_prefix_label_test = 1 *) begin
      $display("PASS my_match");
    end : my_match

    #1;
    if (a !== 1'b1) begin
      $display("FAILED my_blk: Expected 1, got %b", a);
      failed = 1'b1;
    end else begin
      $display("PASS my_blk");
    end

    if (!failed) $display("PASSED");
    done = 1'b1;
  end
endmodule
