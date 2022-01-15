module test;
   reg [19:0] x1;
   initial
     main;
   task main;
      begin
	 x1 = 20'habcde;
	 $display("x1: %h; x2: %h", x1, (x1 - x1 - 1));
      end
   endtask
endmodule // test
