module top;
  parameter ip = 1;
  parameter rp = 2.0;
  parameter sp = "\003";

  real rlval;
  wire real wrval;
  reg [3:0] rval;
  wire [3:0] wval;
  assign wval = 2;

  initial begin
    rval = 4'b1001;
    rlval = 2.0;
    $check_number(1);
    $check_number(ip);
    $check_number(2.0);
    $check_number(rp);
    $check_number("\003");
    $check_number(sp);

    $check_number(rlval);
    $check_number(rlval+1);
    $check_number(wrval);
    $check_number(wrval+1);
    $check_number(rval);
    $check_number(rval+1);
    $check_number(wval);
    $check_number(wval+1);
  end
endmodule
