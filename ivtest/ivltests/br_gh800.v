
// br_gh800 (github Issue #800)
module AssignLiterals;
   initial begin
      string s;

      // Note that the \0 in this string literal assigned to the string
      // must disappear, leaving "AB" what is actually assigned to the
      // string. See IEEE Std 1800-2017: 6.16 String data type.
      s = "A\0B";
      $display("1. '%s' len=%1d hex=%02h%02h%02h",s,s.len(), s[0], s[1], s[2]);
      if (s.len() != 2 || s != "AB" || s[0] !== 'h41 || s[1] !== 'h42) begin
	 $display("FAILED");
	 $finish;
      end

      s = "A\015\12B";
      $display("2. len=%1d hex=%02h%02h%02h%02h",s.len(), s[0], s[1], s[2], s[3]);
      if (s.len() != 4 || s[0] !== 'h41 || s[1] !== 'h0d
	  || s[2] !== 'h0a || s[3] !== 'h42) begin
	 $display("FAILED");
	 $finish;
      end

      s = {"A","\015","\012","B"};
      $display("3. len=%1d hex=%02h%02h%02h%02h",s.len(), s[0], s[1], s[2], s[3]);
      if (s.len() != 4 || s[0] !== 'h41 || s[1] !== 'h0d
	  || s[2] !== 'h0a || s[3] !== 'h42) begin
	 $display("FAILED");
	 $finish;
      end

      s = "A\170\171B";
      $display("4. '%s' len=%1d hex=%02h%02h%02h%02h",s,s.len(),
	       s[0], s[1], s[2], s[3]);
      if (s.len() != 4 || s[0] !== 'h41 || s[1] !== 'h78 || s[2] !== 'h79
	  || s[3] !== 'h42) begin
	 $display("FAILED");
	 $finish;
      end

      s = {"A",8'o15,8'o12,"\012","B"};
      $display("5. len=%1d hex=%02h%02h%02h%02h%02h",s.len(), s[0], s[1], s[2], s[3], s[4]);
      if (s.len() != 5 || s[0] !== 'h41 || s[1] !== 'h0d
	  || s[2] !== 'h0a || s[3] !== 'h0a || s[4] !== 'h42) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
    end
endmodule
