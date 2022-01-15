module bug;

wire a, b, c, d;
assign c = 1'bx;
assign a = 1'b1;
assign b = 1'b0;
assign d = 1'bx;

wire e = {c,d} == {a,b};

initial
  begin
    #2
    if ((e == 1'b1) || (e == 1'b0))
        $display("FAILED -- abcde=%b%b%b%b%b", a, b, c, d, e);
    else
        $display("PASSED");
    $finish;
  end

endmodule
