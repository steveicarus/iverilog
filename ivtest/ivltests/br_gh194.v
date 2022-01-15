
module string_example;

   function int example( string my_string );
      if( my_string[1] != 8'h65 ) begin
	 return 1;
      end else begin
	 return 0;
      end
   endfunction // example

   string test_string;
   initial begin
      test_string = "Hello, World";
      if (test_string[0] !== 8'h48) begin
	 $display("FAILED -- test+string[0] = %h", test_string[0]);
	 $finish;
      end
      if (example(test_string) === 1) begin
	 $display("FAILED -- example(test_string) returned error.");
	 $finish;
      end
      $display("PASSED");
      $finish;
   end
endmodule
