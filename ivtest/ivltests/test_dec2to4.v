// This module generate enable and two-bit selector for verifying a 2-to-4 decoder

module stimulus #(parameter M = 8, T = 10) (
                 output reg [1:0] sel,
                 output reg en
                 );

bit [2:0] i;

initial begin
  sel = 0;
  en = 1'bx;
  #T;
  sel = 1;
  #T;
  sel = 2;
  #T;
  sel = 3;
  #T;
  sel = 2'bxx;
  #T;
  en = 0;
  #T;
  en = 1;
  #T;
  en = 1'bx;
  #T;
  for (i = 0; i < M; i=i+1) begin
    #T;
    {sel, en} = i;
  end

end


endmodule

// This module always checks that y complies with a decoding operation

module check (input [1:0] sel, input en, input [0:3] y);

always @(sel, en, y) begin
     if (en == 0) begin
       #1;
        if (y !== 4'b0000) begin
           $display("ERROR");
           $finish;
        end
      else if (en == 1) begin
        #1;
        case (sel)
          0: if (y !== 4'b1000) begin
               $display("ERROR");
               $finish;
             end
          1: if (y !== 4'b0100) begin
               $display("ERROR");
               $finish;
             end
          2: if (y !== 4'b0010) begin
               $display("ERROR");
               $finish;
             end
          3: if (y !== 4'b0001) begin
               $display("ERROR");
               $finish;
             end
          default: if (y !== 4'b0000) begin
                     $display("ERROR");
                     $finish;
                   end
         endcase
      end // else
    else begin
      if (y !== 4'b0000) begin
           $display("ERROR");
           $finish;
        end
    end
    end // if
end

endmodule


module test;
  parameter M = 8;
  parameter T = 10;
  parameter  S = 4*M*T + 40;

  wire [1:0] sel;
  wire en;
  wire [0:3] y;


  stimulus #(M, T) stim  (.sel(sel), .en(en) );
  dec2to4          duv   (.sel(sel), .en(en), .y(y) );
  check            check (.sel(sel), .en(en), .y(y) );

  initial begin
    #S;
    $display("PASSED");
    $finish;
  end

endmodule
