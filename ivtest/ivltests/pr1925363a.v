module top;
  parameter wid = 9;

  wire [31:0] apass;
  wire [31:0] afail;

  assign apass = {(wid-8){ 8'b0}}; // This will pass.
  assign afail = {(wid-16){8'b0}}; // and this will fail.
endmodule
