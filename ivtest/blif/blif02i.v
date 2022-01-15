
module cmpN
  #(parameter WID = 4)
   (input wire [WID-1:0] A,
    input wire [WID-1:0] B,
    output reg QE, QN, QGT, QGE
    /* */);

   always @(A, B)
     if (A > B)
       QGT = 1;
     else
       QGT = 0;

   always @(A, B)
     if (A >= B)
       QGE = 1;
     else
       QGE = 0;

   always @(A, B)
     if (A == B)
       QE = 1;
     else
       QE = 0;

   always @(A, B)
     if (A != B)
       QN = 1;
     else
       QN = 0;


/*
   always @(A, B)
      if (A > B) begin
	 QE = 0;
	 QN = 1;
	 QGT = 1;
	 QGE = 1;
      end else if (A == B) begin
	 QE = 1;
	 QN = 0;
	 QGT = 0;
	 QGE = 1;
      end else begin
	 QE = 0;
	 QN = 1;
	 QGT = 0;
	 QGE = 0;
      end
*/
endmodule // add
