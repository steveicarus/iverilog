

`define FOO(val=42, text="42") do_foo(val, text)

module main;

   int ref_val;
   string ref_text;
   task do_foo(int val, string text);
      if (val!=ref_val || text!=ref_text) begin
	 $display("FAILED -- val=%d (expect %d), text=%0s, (expect %0s)",
		  val, ref_val, text, ref_text);
	 $finish;
      end
   endtask // do_foo

   initial begin
      ref_val = 42;
      ref_text = "42";
      `FOO(,);

      ref_val = 42;
      ref_text = "41";
      `FOO(,"41");

      ref_val = 41;
      ref_text = "42";
      `FOO(41,);

      ref_val = 41;
      ref_text = "41";
      `FOO(41,"41");

      $display("PASSED");
   end // initial begin

endmodule // main
