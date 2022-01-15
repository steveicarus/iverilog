// pr1661640.v

 module test;
   reg [112:1] hello1, hello2;
   initial begin
      hello1 = "Hello world!\n";
      hello2 = "Hello world!\012";
      main;
   end
   task main;
      begin
	 $write ("%s\n", "Hello world!"); // Ok
	 $write ("%s", "Hello world!\n"); // bad
	 $write ("\nhello1; escaped NL:\n");
	 $write ("%0s", hello1); // bad
	 $write ("%x", hello1);
	 $write ("\nhello2; octal NL:\n");
	 $write ("%0s", hello2); // bad
	 $write ("%x", hello2);
	 $display();
      end
   endtask
 endmodule
