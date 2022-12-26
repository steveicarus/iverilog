// Check that the signedness of package scoped functions is handled correctly,
// when passing the result of the function to a system function

package P;
  function shortint s();
    return -1;
  endfunction

  function bit [15:0] u();
    return -1;
  endfunction
endpackage


module test;

  string s;

  initial begin
    s = $sformatf("%0d %0d", P::s(), P::u());
    if (s == "-1 65535") begin
      $display("PASSED");
    end else begin
      $display("FAILED s=%s", s);
    end
  end

endmodule
