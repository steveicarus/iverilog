module pr3571573;

wire [6:1] top_io;

data_test dut(.io(top_io));

initial begin
  #1 $display("%b", top_io);
end

endmodule

module data_test(inout [6:1] io);

wire [4:1] io1;
wire       io2;
wire       io3;

assign io = {io3, io2, io1};

assign io1[3:2] = 2'b01;

endmodule
