
module main;

   string foo;
   int	  error_count;

   task check_char(input int idx, input [7:0] val);
      if (foo[idx] !== val) begin
	 $display("FAILED: foo[%0d]==%02h, expecting %02h",
		  idx, foo[idx], val);
	 error_count = error_count+1;
      end
   endtask // check_char

   initial begin
      // These are the special charasters in strings as defined by
      // IEEE Std 1800-2017: 5.9.1 Special characters in strings.
      // The string assignment is governed by:
      // IEEE Std 1800-2017: 6.16 String data type
      foo = "abc\n\t\\\"\v\f\a\001\002\x03\x04";
      error_count = 0;

      check_char(0, 8'h61); // 'a'
      check_char(1, 8'h62); // 'b'
      check_char(2, 8'h63); // 'c'
      check_char(3, 8'h0a); // '\n'
      check_char(4, 8'h09); // '\t'
      check_char(5, 8'h5c); // '\\'
      check_char(6, 8'h22); // '\"'
      check_char(7, 8'h0b); // '\v'
      check_char(8, 8'h0c); // '\f'
      check_char(9, 8'h07); // '\a'
      check_char(10, 8'h01); // '\001'
      check_char(11, 8'h02); // '\002'
      check_char(12, 8'h03); // '\x03'
      check_char(13, 8'h04); // '\x04'

      if (foo.len() !== 14) begin
	 $display("FAILED: foo.len() == %0d, should be 14", foo.len());
	 error_count = error_count+1;
      end

      if (error_count == 0) $display("PASSED");
   end
endmodule // main
