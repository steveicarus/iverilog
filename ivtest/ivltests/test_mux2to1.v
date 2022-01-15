// This module generate all 8 inputs for three boolean variables

module stimulus #(parameter M = 8, T = 10) (
                 output reg i0, i1,
                 output reg s
                 );

bit [2:0] i;

initial begin
  for (i = 0; i < M; i=i+1) begin
    #T;
    {i0, i1, s} = i;
  end
  #T;
end


endmodule

// This module always checks the internal generated muxed output complies with the received one

module check (input i0, i1, s, y);

logic y_check;

always @(i0, i1, s)
   y_check = s ? i1 : i0;

always @(y, y_check) begin
     #1 if (y != y_check) begin
       $display("ERROR");
       $finish;
     end
end

endmodule


module test;
  parameter M = 8;
  parameter T = 10;
  parameter  S = (M+1)*T + 40;

  wire i0, i1, s, y;


  stimulus #(M, T) stim  (.i0(i0), .i1(i1), .s(s) );
  mux2to1          duv   (.i0(i0), .i1(i1), .s(s), .y(y) );
  check            check (.i0(i0), .i1(i1), .s(s), .y(y) );

  initial begin
    #S;
    $display("PASSED");
    $finish;
  end

endmodule
