// Check that empty named connections are allowed before and after a wildcard.

module M(input value, output result);
  assign result = value;
endmodule

module test;

  wire value = 1'b1;
  wire plain;
  wire before_wildcard;
  wire after_wildcard;

  M i_plain(.value(), .result(plain));
  M i_before(.value(), .result(before_wildcard), .*);
  M i_after(.*, .value(), .result(after_wildcard));

  initial begin
    #1;
    if (plain !== 1'bz || before_wildcard !== 1'bz ||
        after_wildcard !== 1'bz) begin
      $display("FAILED: plain=%b before=%b after=%b",
               plain, before_wildcard, after_wildcard);
    end else begin
      $display("PASSED");
    end
  end

endmodule
