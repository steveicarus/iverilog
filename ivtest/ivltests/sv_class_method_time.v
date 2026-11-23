// Check $time, $stime and $realtime inside a class method.

class C;
  function realtime get_realtime();
    return $realtime;
  endfunction
  function time get_time();
    return $time;
  endfunction
  function time get_stime();
    return $stime;
  endfunction
endclass

module test;
  C c;
  int failed = 0;

  initial begin
    c = new;
    #10;
    if (c.get_realtime() != 10.0) begin
      $display("FAILED: $realtime=%0t", c.get_realtime());
      failed = 1;
    end
    if (c.get_time() != 10) begin
      $display("FAILED: $time=%0t", c.get_time());
      failed = 1;
    end
    if (c.get_stime() != 10) begin
      $display("FAILED: $stime=%0t", c.get_stime());
      failed = 1;
    end
    if (!failed)
      $display("PASSED");
  end
endmodule
