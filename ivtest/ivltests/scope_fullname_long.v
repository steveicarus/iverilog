// Check that %m works for scopes with hierarchical names longer than 4096
// characters.

module nest #(parameter N = 0) ();
  reg [8*8192-1:0] text;

  generate
    if (N > 0) begin : g_with_a_deliberately_long_generate_name
      nest #(N-1) u_with_a_deliberately_long_instance_name();
    end
  endgenerate

  initial if (N == 0) begin
    $sformat(text, "%m");
    if (text != {"test.top", {60{".g_with_a_deliberately_long_generate_name",
                                 ".u_with_a_deliberately_long_instance_name"}}})
      $display("FAILED: got %0s", text);
    else
      $display("PASSED");
  end
endmodule

module test;
  nest #(60) top();
endmodule
