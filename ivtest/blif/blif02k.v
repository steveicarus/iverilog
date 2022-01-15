
module test_mux
   (input wire [1:0] D0, D1,
    input wire [1:0] S,
    output reg [1:0] Q, R);

   always @(*) begin
      if (S[1]==1'b0)
	case (S[0])
	  1'b0: Q = D0;
	  1'b1: Q = D1;
	endcase // case (S[0])
      else
	Q = 2'b0;

      case (S[1])
	1'b0: if (S[0])
	  R = D1;
	else
	  R = D0;
	1'b1: R = 2'b00;
      endcase
   end

endmodule // test_mux
