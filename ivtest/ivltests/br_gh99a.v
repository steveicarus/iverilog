module test();

wire [7:0] value;
wire       pass;

assign value[3:0] = 4'd2;

assign pass = (value === 8'bzzzz0010);

initial begin
  #2 $display("%b %b", value, pass);
  if (pass === 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
