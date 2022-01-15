
module test_mux
   (input wire [1:0] D0, D1,
    input wire [1:0] S,
    output reg [1:0] Q);

   always @(*) begin
      case (S)
	2'b00: Q = D0;
	2'b01: Q = D1;
	default: Q = 0;
      endcase // case (S)
   end

endmodule // test_mux
