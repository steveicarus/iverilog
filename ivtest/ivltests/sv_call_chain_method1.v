// Chained call: function returns class handle, then method on result (a().b()).

module test;

  class C;
    function int f;
      f = 7;
    endfunction
  endclass

  function C get_c;
    get_c = new;
  endfunction

  initial begin
    int x;
    x = get_c().f();
    if (x !== 7) begin
      $display("FAILED");
    end else begin
      $display("PASSED");
    end
  end

endmodule
