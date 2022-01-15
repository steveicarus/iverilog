
module foo_mod(output reg pass_flag, input wire go_flag);

   typedef enum logic [1:0] { W0, W1, W2 } foo_t;
   foo_t foo;

   always @(posedge go_flag) begin

      pass_flag = 0;

      if ($bits(foo) !== 2) begin
	 $display("FAILED -- $bits(foo)=%0d", $bits(foo));
	 $finish;
      end

      if ($bits(foo_t) !== 2) begin
	 $display("FAILED -- $bits(foo_t)=%0d", $bits(foo_t));
	 $finish;
      end

      pass_flag = 1;
   end

endmodule

module main;
   logic go_flag = 0;
   wire [1:0] pass_flag;

   foo_mod dut[1:0] (.pass_flag(pass_flag), .go_flag(go_flag));

   initial begin
      #1 go_flag = 1;
      #1 if (pass_flag !== 2'b11) begin
	 $display("FAILED -- pass_flag=%b", pass_flag);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
