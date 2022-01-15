module top (
	    );

reg signed [13:0] datain;
wire signed [15:0] dataout;

assign dataout = datain <<< 2;

reg test_failed;

initial
  begin
     test_failed = 0;
     #1 datain = 14'h0FFF;
     #1 datain = 14'h0000;
     #1 datain = 14'h1FFF;
     #1 datain = 14'h1000;
     #1 datain = 14'h2FFF;
     #1 datain = 14'h2000;
     #1 datain = 14'h3FFF;
     #1 datain = 14'h3000;
     #2;
     if (test_failed)
       $display("TEST FAILED :-(");
     else
       $display("TEST PASSED :-)");
  end

wire signed [15:0] expected_dataout;

assign expected_dataout = $signed({datain[13:0], 2'b0});

always @(dataout)
  if (expected_dataout != dataout)
    begin
       $display("datain = %d dataout = %d expected = %d ... CHECK FAILED", datain, dataout, expected_dataout);
       test_failed = 1;
    end
  else
    $display("datain = %d dataout = %d expected = %d ... CHECK PASSED", datain, dataout, expected_dataout);

endmodule // top
