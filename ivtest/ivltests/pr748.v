module signed_multiplier_test;

   reg failed_flag = 0;
   reg  signed [5:0] s_prod;

   wire        [2:0] u_pos_two = 3'b010;
   wire signed [2:0] s_pos_two = 3'sb010;
   wire signed [2:0] s_neg_two = 3'sb110;

   wire s = 1'b1;	// flag to indicate signed
   wire u = 1'b0;	// flag to indicate unsigned

   initial begin

      // unsigned positive two as first argument of multiply

      #1 s_prod = u_pos_two * u_pos_two;
      check_mult(1,u,u_pos_two,u,u_pos_two,s_prod,6'sb000100);

      #1 s_prod = u_pos_two * s_pos_two;
      check_mult(2,u,u_pos_two,s,s_pos_two,s_prod,6'sb000100);

      // This makes an unsigned result.
      #1 s_prod = u_pos_two * s_neg_two;
      check_mult(3,u,u_pos_two,s,s_neg_two,s_prod,6'sb001100);

      // signed positive two as first argument of multiply

      #1 s_prod = s_pos_two * u_pos_two;
      check_mult(4,s,s_pos_two,u,u_pos_two,s_prod,6'sb000100);

      #1 s_prod = s_pos_two * s_pos_two;
      check_mult(5,s,s_pos_two,s,s_pos_two,s_prod,6'sb000100);

      #1 s_prod = s_pos_two * s_neg_two;
      check_mult(6,s,s_pos_two,s,s_neg_two,s_prod,6'sb111100);

      // signed negative two as first argument of multiply

      // This makes an unsigned result.
      #1 s_prod = s_neg_two * u_pos_two;
      check_mult(7,s,s_neg_two,u,u_pos_two,s_prod,6'sb001100);

      #1 s_prod = s_neg_two * s_pos_two;
      check_mult(8,s,s_neg_two,s,s_pos_two,s_prod,6'sb111100);

      #1 s_prod = s_neg_two * s_neg_two;
      check_mult(9,s,s_neg_two,s,s_neg_two,s_prod,6'sb000100);

      if (failed_flag == 0)
	$display("PASSED");

      $finish;
   end

   task check_mult;
      input [31:0] idx;
      input        signeda;
      input [ 2:0] arga;
      input        signedb;
      input [ 2:0] argb;
      input [ 5:0] result,expected;

      if (result !== expected) begin
	 failed_flag = 1;
         $write("failed: test %0d, ",idx);

         if (signeda)
	    $write("3'sb%b",arga);
	 else
	    $write("3 'b%b",arga);

         $write(" * ");

         if (signedb)
	    $write("3'sb%b",argb);
	 else
	    $write("3 'b%b",argb);

         $write(" = 6'sb%b (expected 6'sb%b)\n",result,expected);
      end
   endtask

endmodule
