module top;
  real rvar1, rvar2, rtmp;
  wire real wrcon3, wrcon4, wrcon5, wrcon6;

  wire real wrcon1 = {2.0, 1.0};
  wire real wrcon2 = {rvar1, rvar2};

  assign wrcon3 = {2.0, 1.0};
  assign wrcon4 = {rvar1, rvar2};

  assign {wrcon5, wrcon6} = 1.0;

  initial begin
    rtmp = {2.0, 1.0};
    rtmp = {rvar1, rvar2};

    {rvar1, rvar2} = rtmp;
  end
endmodule
