
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

      obj = new[3];
      obj = '{1,2,3};
      foo = sum_array(obj);
      if (foo != 6.0) begin
	 $display("FAILED -- sum of '{%f,%f,%f} is %0d", obj[0], obj[1], obj[2], foo);
	 $finish;
      end

      obj = new[3] ('{4,5,6});
      foo = sum_array(obj);
      if (foo != 15.0) begin
	 $display("FAILED -- sum of '{4,5,6} is %0d", foo);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endprogram // main
