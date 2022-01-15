module top;
  reg [8:1] val;
  wire [1:4] wval;
  reg [3:0] wdrv;
  real r_arr [1:8];
  integer i_arr [8:1];
  integer lp;

  assign wval = wdrv;

  initial begin
    wdrv = 4'b1010;
    for (lp=1; lp<=8; lp=lp+1) begin
      val[lp] = lp % 2;
      r_arr[lp] = lp + 0.25;
      i_arr[lp] = lp - 1;
    end

    #1;
    $check_val(val, 3, 1);
    $check_val(wval, 4, 0);
    $check_val(r_arr, 5, 5.25);
    $check_val(i_arr, 2, 1);
    $display("Original value is %b", val);
    $put_val(val, 2, 1);
    $put_val(val, 1, 0);
    $display("     New value is %b", val);
    $display("Original net value is %b", wval);
    $put_val(wval, 2, 1);
    $put_val(wval, 1, 0);
    $display("     New net value is %b", wval);
    #1;
    // Verify that an update overrides the put value
    wdrv = 4'b1001;
    $display("     net value is now %b", wval);
  end

endmodule
