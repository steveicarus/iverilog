module main;
   reg signed [7:0] a;
   reg [7:0] b;
   initial begin
      // Make sure the arithmetic right shift sign extends
      $display("simple arithmetic right shift");
      a = 8'b11001001;
      $display("before:  a = %b", a);
      a = a>>>1;
      $display("after:   a = %b", a);
      if (a !== 8'b11100100) begin
	 $display("FAILED");
	 $finish;
      end

      // The concatenation operator is always unsigned, so
      // it must turn off sign extension.
      $display("concatenated arithmetic right shift");
      a = 8'b11001001;
      b = 0;
      $display("before:  a = %b", a);
      {a,b} = {a,b}>>>1;
      $display("after:   a = %b", a);
      if (a !== 8'b01100100) begin
	 $display("FAILED");
	 $finish;
      end


      // The concatenation operator is always unsigned, but
      // we can turn on signed behavior with $signed.
      $display("concatenated arithmetic right shift with $signed");
      a = 8'b11001001;
      b = 0;
      $display("before:  a = %b", a);
      {a,b} = $signed({a,b})>>>1;
      $display("after:   a = %b", a);
      if (a !== 8'b11100100) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end
endmodule
