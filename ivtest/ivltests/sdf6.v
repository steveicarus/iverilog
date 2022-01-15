module dff (q, d, cp, sdn, cdn);
  output q;
  input cp;
  input d;
  input sdn;
  input cdn;

  reg q;

  always @(posedge cp or negedge sdn or negedge cdn) begin
   if (~sdn) q <= 1;
   else if (~cdn) q <= 0;
   else q <= d;
  end

  specify
    if (sdn && cdn) (posedge cp => (q +: d)) = (1, 1);
    (negedge cdn => (q +: 1'b0)) = (1, 1);
    (negedge sdn => (q -: 1'b1)) = (1, 1);
  endspecify
endmodule

module test;
  reg d, clk, set, clr;

  dff dut(q, d, clk, ~set, ~clr);

  initial begin
    d=0; clk=0; set=0; clr=0;
    $monitor($time,, "d=%b, clk=%b, set=%b, clr=%b, q=%b",
             d, clk, set, clr, q);
    $sdf_annotate("ivltests/sdf6.sdf");
    #10 d = 1;
    #10 set = 1;
    #10 set = 0;
    #10 clr = 1;
    #10 clr = 0;
    #10 clk = 1;
    #10 d = 0;
  end
endmodule
