module multiplexer(
  input [3:0] din,
  input [1:0] sel,
  output dout
);

function func(input [3:0] in, input [1:0] s);
  begin
    if(s==2'h0)
      func=in[0];
    if(s==2'h1)
      func=in[1];
    if(s==2'h2)
      func=in[2];
    if(s==2'h3)
      func=in[3];
  end
endfunction

assign dout = func(din,sel);

endmodule

module multiplexer_test;

wire [3:0] din = 4'b1001;
reg  [1:0] sel;
wire       dout;

multiplexer mux(din, sel, dout);

reg failed = 0;

initial begin
  $monitor($time,,sel,,dout);
  #0 sel=2'b00;
  #0 if (dout !== 1'b1) failed = 1;
  #2 sel=2'b10;
  #0 if (dout !== 1'b0) failed = 1;
  #2 sel=2'bxx;
  #0 if (dout !== 1'b0) failed = 1;
  #2 sel=2'bzz;
  #0 if (dout !== 1'b0) failed = 1;
  #1;
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
