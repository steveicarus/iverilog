/* pr1636409 */
module top;
  wire [3:0] fail, good;
  wire eni;
  reg [2:0] rg;
  reg in, en, clk;

  assign #1 eni = en;
  assign #1 fail = (eni) ? {rg,in} : 'b0;
  assign #1 good = {4{eni}} & {rg,in};

  always @(fail or good or eni) begin
    $strobe("fail=%b, good=%b, en=%b at %0t", fail, good, eni, $time);
  end

  always #10 clk = ~clk;

   always @(posedge clk) begin
     en = ~en;
     in = ~in;
     rg = ~rg;
   end

  initial begin
//    $dumpfile("results.vcd");
//    $dumpvars(0, top);
    clk = 0;
    en = 0;
    in = 0;
    rg = 3'b101;
    #50 $finish(0);
  end
endmodule
