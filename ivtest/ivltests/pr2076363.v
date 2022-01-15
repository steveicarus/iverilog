module top;
  reg [10*8-1:0] str [2:0];
  reg [31:0] idx [2:0];
  reg [4*8-1:0] pvstr, pvstr2;
  reg [15:0] pvidx, pvidx2, pvbase;

  initial begin
    pvstr = "S";
    pvstr2 = "SF";
    pvidx = 'd2;
    pvidx2 = 'd8;
    pvbase = 'd0;
    str[0] = "FAIL";
    str[1] = "PA";
    str[2] = "ED";
    idx[0] = 0;
    idx[1] = 1;
    idx[2] = 2;
    $write("%0s", str[idx[1]]); // This prints PA or FAIL.
    $write("%0s", pvstr[idx[0] +: 16]); // This adds an S.
    $write("%0s", pvstr2[pvidx2[pvbase +: 4] +: 8]); // This adds another S.
    $display("%0s", str[pvidx[pvbase +: 8]]); // This adds the ED.
  end
endmodule
