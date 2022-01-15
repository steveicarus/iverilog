module test();
   parameter N_N = 3;

    reg signed [2*N_N-1:0] val_neg;
    reg signed [2*N_N-1:0] val_pos;


   initial
     begin
	val_neg =  {{N_N+1{1'b1}},{N_N-1{1'b0}}};
	val_pos =  {{N_N+1{1'b0}},{N_N-1{1'b1}}};
	#1 $display("Var %d vs signed(concat) %d",
		    val_neg,
		    $signed({{N_N+1{1'b1}},{N_N-1{1'b0}}}));
	$finish(0);
     end // initial begin
endmodule // test
