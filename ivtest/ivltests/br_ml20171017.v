module test(output [7:0] dataout[1:0]);

assign dataout[0] = 8'h55;
assign dataout[1] = 8'haa;

initial begin
  #0;
  if (dataout[0] === 8'h55 && dataout[1] === 8'haa)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
