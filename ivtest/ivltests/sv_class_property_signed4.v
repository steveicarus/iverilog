// Check that the signedness of class properties are handled correctly when
// accessing the property in a class method and passing it to a system function.

module test;

  class C;
    shortint s = -1;
    bit [15:0] u = -1;

    task test;
      string str;

      str = $sformatf("%0d %0d", s, u);
      if (str == "-1 65535") begin
        $display("PASSED");
      end else begin
        $display("FAILED s=%s", s);
      end
    endtask

  endclass

  C c;

  initial begin
    c = new;
    c.test();
  end

endmodule
