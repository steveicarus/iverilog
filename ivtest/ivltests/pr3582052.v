module bug;

reg [7:0] Data[0:3][0:15];

reg [7:0] Expect0;
reg [7:0] Expect1;
reg [7:0] Expect2;
reg [7:0] Expect3;

reg [7:0] Actual0;
reg [7:0] Actual1;
reg [7:0] Actual2;
reg [7:0] Actual3;

integer i;
integer j;

reg Failed = 0;

initial begin
  for (i = 0; i < 4; i = i + 1) begin
    for (j = 0; j < 16; j = j + 1) begin
      Data[i][j] = (i << 4) + j;
    end
  end
  // this catches the original bug
  for (j = 0; j < 16; j = j + 1) begin
    Expect0 = 0*16 + j;  Actual0 = Data[0][j];
    Expect1 = 1*16 + j;  Actual1 = Data[1][j];
    Expect2 = 2*16 + j;  Actual2 = Data[2][j];
    Expect3 = 3*16 + j;  Actual3 = Data[3][j];
    $display("%h %h %h %h", Actual0, Actual1, Actual2, Actual3);
    if (Actual0 !== Expect0) Failed = 1;
    if (Actual1 !== Expect1) Failed = 1;
    if (Actual2 !== Expect2) Failed = 1;
    if (Actual3 !== Expect3) Failed = 1;
  end
  // extended tests to check the bug fix doesn't break anything else
  for (i = 0; i < 4; i = i + 1) begin
    Expect0 = i*16 + 0;  Actual0 = Data[i][0];
    Expect1 = i*16 + 3;  Actual1 = Data[i][3];
    Expect2 = i*16 + 6;  Actual2 = Data[i][6];
    Expect3 = i*16 + 9;  Actual3 = Data[i][9];
    $display("%h %h %h %h", Actual0, Actual1, Actual2, Actual3);
    if (Actual0 !== Expect0) Failed = 1;
    if (Actual1 !== Expect1) Failed = 1;
    if (Actual2 !== Expect2) Failed = 1;
    if (Actual3 !== Expect3) Failed = 1;
  end
  Expect0 = 0*16 + 0;  Actual0 = Data[0][0];
  Expect1 = 0*16 + 9;  Actual1 = Data[0][9];
  Expect2 = 3*16 + 0;  Actual2 = Data[3][0];
  Expect3 = 3*16 + 9;  Actual3 = Data[3][9];
  $display("%h %h %h %h", Actual0, Actual1, Actual2, Actual3);
  if (Actual0 !== Expect0) Failed = 1;
  if (Actual1 !== Expect1) Failed = 1;
  if (Actual2 !== Expect2) Failed = 1;
  if (Actual3 !== Expect3) Failed = 1;
  if (Failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
