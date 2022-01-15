
program main;

   function real sum_array(real array[]);
      int idx;
      sum_array = 0.0;
      for (idx = 0 ; idx < array.size() ; idx = idx+1)
	sum_array = sum_array + array[idx];
   endfunction // sum_array

   real obj[];
   real foo;
   initial begin
      foo = sum_array('{});
      if (foo != 0.0) begin
	 $display("FAILED -- sum of empty array returns %0d", foo);
	 $finish;
      end

      obj = new[3] (3.0);
      foo = sum_array(obj);
      if (foo != 9.0) begin
	 $display("FAILED -- sum of '{3.3.3} is %0d", foo);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endprogram // main
