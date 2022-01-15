module test;

  reg signed [31:0] mydata;


  initial
    begin
      mydata = -6;
      repeat ( 11 )
        begin
          add_one(mydata);
          $display("mydata = %0d", mydata);
        end
      $finish(0);
    end


task add_one;
  inout    signed [31:0] myotherdata;
  begin
    myotherdata = myotherdata + 1;
  end
endtask

endmodule
