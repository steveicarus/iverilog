task hello1;
  begin
    $display("hello1 from unit 2");
  end
endtask


task hello2;
  begin
    $display("hello2 from unit 2");
  end
endtask


task hello3;
  begin
    $display("hello3 from unit 2");
  end
endtask


module m3();

  initial begin
    #2; // allow m1 to go first
    m2.m1inst.obj.hello2;
    hello1;
    hello2;
    hello3;
    hello4;
  end

  task hello2;
    begin
      $display("hello2 from m3");
    end
  endtask

endmodule


module m4();

  m3 m3inst();

  task hello1;
    begin
      $display("hello1 from m4");
    end
  endtask

  task hello2;
    begin
      $display("hello2 from m4");
    end
  endtask

  task hello3;
    begin
      $display("hello3 from m4");
    end
  endtask

  task hello4;
    begin
      $display("hello4 from m4");
    end
  endtask

endmodule
