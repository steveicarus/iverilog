// Check that matching null case items terminate constant-function case selection.

module test;

  reg failed;

  function integer select;
    input real value;
    begin
      select = 7;
      case (value)
        // Check that the search stops at the first label.
        0.0: ;
        0.0: select = 8;
        default: select = 9;
      endcase
    end
  endfunction

  localparam integer matched = select(0.0);
  localparam integer unmatched = select(1.0);

  `define check(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    `check(matched, 7);
    `check(unmatched, 9);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
