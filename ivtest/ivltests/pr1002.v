module top (
	    );

reg signed [13:0] datain;
wire signed [15:0] dataout;

assign dataout = datain >>> 2;

reg test_failed;

wire signed [15:0] expected_dataout;

assign expected_dataout = ($signed({datain[13:2], 2'b0}) / 4) ;

task check_data;
  if (expected_dataout != dataout)
    begin
       $display("datain = %d dataout = %h expected = %h ... CHECK FAILED", datain, dataout, expected_dataout);
       test_failed = 1;
    end
  else
    $display("datain = %d dataout = %d expected = %d ... CHECK PASSED", datain, dataout, expected_dataout);
endtask

initial
  begin
     test_failed = 0;
     #1 datain = 14'h0FFF;
     #0 check_data; // #0 delay to allow the wire to resolve
     #1 datain = 14'h0000;
     #0 check_data;
     #1 datain = 14'h1FFF;
     #0 check_data;
     #1 datain = 14'h1000;
     #0 check_data;
     #1 datain = 14'h2FFF;
     #0 check_data;
     #1 datain = 14'h2000;
     #0 check_data;
     #1 datain = 14'h3FFF;
     #0 check_data;
     #1 datain = 14'h3000;
     #0 check_data;
     #2;
     if (test_failed)
       $display("TEST FAILED :-(");
     else
       $display("TEST PASSED :-)");
  end

endmodule // top
