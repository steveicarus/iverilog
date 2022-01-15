program test;

  class a;
    function new(string str);
      $display(str);
    endfunction
  endclass // a

  initial begin
    a m_a;
    m_a = new("PASSED");
  end

endprogram
