module main;

   wire a;
   device U1(a);

   task work;
     begin
	$deposit(U1.r, 1);
	$display("PASSED");
	$finish;
     end
   endtask

   initial work;

endmodule

module device(r);
   output r;
   reg r;
endmodule
