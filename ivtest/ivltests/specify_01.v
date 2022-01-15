module dff (clk, d, q) ;

input d,clk;
output q;

reg q_out;
wire q;

specify
  specparam tR_clk_q = 100,
            tF_clk_q = 150;

  (clk,d => q) = (tR_clk_q,tF_clk_q);
endspecify

always @(posedge clk)
   q_out <= d;

buf u_buf (q,q_out);

endmodule

module test;

reg clk, d;
reg err;
time pos_lvl,neg_lvl;

dff u_dff (clk,d,q);

initial
  begin
//    $dumpfile("test.vcd");
//    $dumpvars(0,test);

    err = 0;
    d = 0;
    clk = 0;
    #200;
    clk = 1;
    #200;
    clk = 0;
    #200;
    clk = 1;
    #200;
    clk = 0;
    #200;
    clk = 1;
    $display("pos_lvl=%t neg_lvl=%t",pos_lvl,neg_lvl);
    if((pos_lvl != 700) && (neg_lvl != 350))
     $display("FAILED");
    else
     $display("PASSED");
  end

always @(posedge q)
   pos_lvl = $time;

always @(negedge q)
   neg_lvl = $time;

initial
  begin
    #250;
    d = 1;
    #450;
    d = 0;
  end



endmodule
