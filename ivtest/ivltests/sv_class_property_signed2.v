// Check that the signedness of class properties are handled correctly when
// accessing the property on a class object and passing it to a system function.

module test;

  class C;
    shortint s = -1;
    bit [15:0] u = -1;
  endclass

  C c;
  string s;

  initial begin
    c = new;

    s = $sformatf("%0d %0d", c.s, c.u);
    if (s == "-1 65535") begin
      $display("PASSED");
    end else begin
      $display("FAILED s=%s", s);
    end
  end

endmodule
