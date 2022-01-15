module switch (q,a,b,c,d,sel);
input a,b,c,d;
input [1:0] sel;
output q;
reg q;
always @ *
     case (sel)
         2'b00: q = a;
	 2'b01: q = b;
	 2'b10: q = c;
	 2'b11: q = d;
     endcase

endmodule

module test ;
reg [1:0] sel;
reg a,b,c,d;
wire q;

switch u_switch (q,a,b,c,d,sel);

initial
  begin
    a = 0;
    b = 0;
    c = 0;
    d = 0;
    sel = 2'b00;

    #1;
    if(q !== 1'b0)
      begin
        $display("FAILED");
        $finish;
      end
    a = 1;
    #1;
    if(q !== 1'b1)
      begin
        $display("FAILED");
        $finish;
      end
    $display("PASSED");
  end
endmodule
