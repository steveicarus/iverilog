// pr1645277

 module test;
   initial main;

   task main;
      integer foo;
      begin
	 foo = 0;
	 while(foo < 5) begin: inner
	    foo = foo + 1;
	 end
	 $write("expected %d; got %d\n", 5, foo);
      end
   endtask
 endmodule
