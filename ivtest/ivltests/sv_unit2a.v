function int hello(int a);
  begin
    $display("hello from unit 1");
    hello = a;
  end
endfunction


task hello1;
  begin
    $display("hello1 from unit 1");
  end
endtask


task hello2;
  begin
    $display("hello2 from unit 1");
  end
endtask


task hello3;
  begin
    $display("hello3 from unit 1");
  end
endtask


class c1;

  task hello2;
    begin
      hello1;
      $display("hello2 from c1");
    end
  endtask

endclass


module m1();

  int i;

  c1 obj;

  initial begin
    #1;
    i = $unit::hello(1);
    obj = new;
    obj.hello2;
    hello1;
    hello2;
    hello3;
    hello4;
  end

  task hello2;
    begin
      $display("hello2 from m1");
    end
  endtask

endmodule


module m2();

  m1 m1inst();

  task hello1;
    begin
      $display("hello1 from m2");
    end
  endtask

  task hello2;
    begin
      $display("hello2 from m2");
    end
  endtask

  task hello3;
    begin
      $display("hello3 from m2");
    end
  endtask

  task hello4;
    begin
      $display("hello4 from m2");
    end
  endtask

endmodule
