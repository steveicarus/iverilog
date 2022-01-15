// pr1755593

module main;

   wire out;
   reg [4:0] data;
   reg [2:0] sel;

   test U1(out, data[0], data[1], data[2], data[3], sel[0], sel[1]);

   initial begin
      for (sel=0 ; sel<4 ; sel=sel+1)
	 for (data=0 ; data<16 ; data=data+1) begin
	    #1 if (out !== data[sel]) begin
	       $display("FAILED -- data=%b, sel=%d", data, sel);
	       $finish;
	    end
	 end

      $display("PASSED");
   end

endmodule // main

module test (Z, D0, D1, D2, D3, E0, E1);

   output Z;
   input D0;
   input D1;
   input D2;
   input D3;
   input E0;
   input E1;

   u_test I48 (Z, D0, D1, D2, D3, E0, E1);

endmodule
primitive u_test (Z, D0, D1, D2, D3, E0, E1);

   output Z;
   input  D0, D1, D2, D3, E0, E1;

   table

         0  ?  ?  ?  0  0  :  0  ;
         1  ?  ?  ?  0  0  :  1  ;

         ?  0  ?  ?  1  0  :  0  ;
         ?  1  ?  ?  1  0  :  1  ;

         ?  ?  0  ?  0  1  :  0  ;
         ?  ?  1  ?  0  1  :  1  ;

         ?  ?  ?  0  1  1  :  0  ;
         ?  ?  ?  1  1  1  :  1  ;

         0  0  ?  ?  x  0  :  0  ;
         1  1  ?  ?  x  0  :  1  ;

         ?  ?  0  0  x  1  :  0  ;
         ?  ?  1  1  x  1  :  1  ;

         0  ?  0  ?  0  x  :  0  ;
         1  ?  1  ?  0  x  :  1  ;

         ?  0  ?  0  1  x  :  0  ;
         ?  1  ?  1  1  x  :  1  ;

         0  0  0  0  x  x  :  0  ;
         1  1  1  1  x  x  :  1  ;

   endtable

endprimitive
