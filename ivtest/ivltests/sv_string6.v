// Test the various string.Xtoa() methods

module testbench;
   string str;
   int	  val;
   real   valr;


   task test_string_value(string str, string reference);
      if (str != reference) begin
	 $display("FAILED -- str=%0s, should be %s", str, reference);
	 $finish;
      end
   endtask // test_string_value

   initial begin
      val = 11;
      valr = 11.1;

      str.itoa(val);
      test_string_value(str, "11");

      str.hextoa(val);
      test_string_value(str, "b");

      str.octtoa(val);
      test_string_value(str, "13");

      str.bintoa(val);
      test_string_value(str, "1011");

      str.realtoa(valr);
      test_string_value(str, "11.1");

      val = -11;
      valr = -11.1;

      str.itoa(val);
      test_string_value(str, "-11");

      str.hextoa(val);
      test_string_value(str, "-b");

      str.octtoa(val);
      test_string_value(str, "-13");

      str.bintoa(val);
      test_string_value(str, "-1011");

      str.realtoa(valr);
      test_string_value(str, "-11.1");

      $display("PASSED");

   end
endmodule
