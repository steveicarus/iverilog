module pr3499807();

supply0 gnd;
wire    net1;
wire    net2;

tranif0 #(100) sw1(gnd, net1, gnd);
tranif0 #(200) sw2(gnd, net2, gnd);

initial begin
  $monitor($time,, gnd,, net1,, net2);
  #300;
  $finish(0);
end

endmodule
