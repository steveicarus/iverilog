// Check that the signedness of methods on user defined classes is handled
// correctly when passing the result to a system function.

module test;

  class C;
    function shortint s;
      return -1;
    endfunction

    function bit [15:0] u;
      return -1;
    endfunction
  endclass

  C c;
  string s;

  initial begin
    c = new;

    s = $sformatf("%0d %0d", c.s(), c.u());
    if (s == "-1 65535") begin
      $display("PASSED");
    end else begin
      $display("FAILED s=%s", s);
    end
  end

endmodule
