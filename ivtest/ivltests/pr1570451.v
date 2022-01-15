module test;

  reg [1:0] bus;
  reg [1:0] skewed_bus;

  integer delay0;  initial delay0 = 5;
  integer delay1;  initial delay1 = 10;

  /* attempt to model skew across the bus using transport delays */

  always @( bus[0] )
    begin
      skewed_bus[0] <= #delay0 bus[0];
    end

   always @( bus[1] )
    begin
      skewed_bus[1] <= #delay1 bus[1];
    end

   initial begin
      #1 bus = 2'b00;
      #11 if (skewed_bus !== 2'b00) begin
	 $display("FAILED -- setup failed.");
	 $finish;
      end

      bus = 2'b11;

      #4 if (skewed_bus !== 2'b00) begin
	 $display("FAILED -- changed far too soon");
	 $finish;
      end

      #2 if (skewed_bus !== 2'b01) begin
	 $display("FAILED -- partial change not right.");
	 $finish;
      end

      #5 if (skewed_bus !== 2'b11) begin
	 $display("FAILED -- final change not right");
	 $finish;
      end

      $display("PASSED");
   end
endmodule
