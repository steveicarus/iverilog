module test;

   reg a, b1, b2;

   submod m1 (a, b1, c1);
   submod m2 (a, b2, c2);

   task set;
      input [2:0] bits;
      reg t1;
      begin
	 t1 <= a;
	 #1 {a,b1,b2} <= bits;
      end
   endtask

   initial
     begin
	$dumpfile("work/vcd-dup.vcd");
	$dumpvars(2, test);             // test, test.m1, test.m2
	$dumpvars(3, m2.c1, m1.mm1.c1); // duplicate signals
	#0;                             // does not trip $enddefinitions
	a = 0;                          // does not trip $enddefinitions
	$dumpvars(0, m1);               // (test.m1), test.m1.mm1, test.m1.mm2
	#1;                             // $enddefinitions called
	$dumpvars(0, m2);               // ignored
     end

   initial
     begin
	#1 set(3'd 0);
	#1;
	#1 set(3'd 1);
	#1;
	#1 set(3'd 2);
	#1 $dumpoff;
	#1 set(3'd 3);
	#1;
	#1 set(3'd 4);
	#1 $dumpon;
	#1 set(3'd 5);
	#1;
	#1 set(3'd 6);
	#1;
	#1 set(3'd 7);
	#1;
	#1 set(3'd 0);
	#1 $dumpall;
	#1 $finish;
     end

endmodule

module submod (a, b, c);

   input a, b;
   output c;

   subsub mm1 (a&b, c1);
   subsub mm2 (a|b, c2);

   assign c = c1 ^ c2;

endmodule

module subsub (a, c);

   input a;
   output c;

   wire c1 = ~a;

   assign c = c1;

endmodule
