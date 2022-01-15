/***********************************************************************

  Array access test cases
  Copyright (C) 2001  Eric LaForest, ecl@pet.dhs.org
  Licenced under GPL

***********************************************************************/

module wire_test_case (array_out, clock, reset);
	output [15:0] array_out;
	input clock, reset;

	reg [3:0] readptr;
	reg [15:0] body [15:0];

	wire [15:0] array_out;
	assign array_out = body[readptr];

//	reg [15:0] array_out;
//	always @(readptr or body[readptr]) begin
//		array_out <= body[readptr];
//	end

	always @(posedge clock) begin
		if (reset == 0) begin
			readptr <= 16'h0000;
			body[0] <= 16'h0001; // Fibonnacci
			body[1] <= 16'h0002;
			body[2] <= 16'h0003;
			body[3] <= 16'h0005;
			body[4] <= 16'h0008;
			body[5] <= 16'h000D;
			body[6] <= 16'h0015;
		end
		else begin
			readptr <= readptr + 16'h0001;
		end
	end
endmodule

module always_test_case (array_out, clock, reset);
	output [15:0] array_out;
	input clock, reset;

	reg [3:0] readptr;
	reg [15:0] body [15:0];

//	wire [15:0] array_out;
//	assign array_out = body[readptr];

	reg [15:0] array_out;
	always @(readptr or body[readptr]) begin
		array_out <= body[readptr];
	end

	always @(posedge clock) begin
		if (reset == 0) begin
			readptr <= 16'h0000;
			body[0] <= 16'h0001; // Fibonnacci
			body[1] <= 16'h0002;
			body[2] <= 16'h0003;
			body[3] <= 16'h0005;
			body[4] <= 16'h0008;
			body[5] <= 16'h000D;
			body[6] <= 16'h0015;
		end
		else begin
			readptr <= readptr + 16'h0001;
		end
	end
endmodule

module BENCH ();
	wire [15:0] array_out1, array_out2;
	reg clock, reset;

	integer count;
        integer errors;

	wire_test_case usingwire (array_out1, clock, reset);
	always_test_case usingalways (array_out2, clock, reset);

	initial begin
//		$dumpfile("waves.vcd");
//		$dumpvars(0, BENCH);
		clock <= 0;
		reset <= 0;
		count <= 0;
		#1000;
	        if (errors == 0)
		  $display("PASSED");
		$finish;
	end

	always begin
		# 10 clock <= ~clock;
	end

	always @(posedge clock) begin
		count <= count + 1;
		case (count)
			10: begin
				reset <= 1;
			end
		endcase
	end

   initial errors = 0;

   always @(negedge clock)
     if (array_out1 !== array_out2)
       begin
	  $display("FAILED: %b !== %b", array_out1, array_out2);
	  errors = errors + 1;
       end

endmodule
