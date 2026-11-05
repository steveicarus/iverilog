// Check that a case-generate default does not require a colon.

module M #(parameter SELECT = 0) (output [1:0] value);

  generate
    case (SELECT)
      0: assign value[0] = 1'b0;
      default assign value[0] = 1'b1;
    endcase
    case (SELECT)
      0: assign value[1] = 1'b0;
      default begin : g
        if (1) begin : nested
          assign value[1] = 1'b1;
        end
      end
    endcase
  endgenerate

endmodule

module test;

  wire [1:0] matched, unmatched;

  M #(0) i_matched(matched);
  M #(1) i_unmatched(unmatched);

  initial begin
    #1;
    if (matched !== 2'b00 || unmatched !== 2'b11) begin
      $display("FAILED: matched=%b unmatched=%b", matched, unmatched);
    end else begin
      $display("PASSED");
    end
  end

endmodule
