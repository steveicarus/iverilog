module top;
  // These should create two ufunc calls each and the zero replication one
  // will not be connected to anything.
  wire [31:0] var1 = {{0{ufunc(0)}}, ufunc(0)};
  wire [31:0] var2 = {ufunc(0), {0{ufunc(0)}}};
  integer fres;

  initial begin
    #1;
    if (fres != 4) $display("FAILED, expected fres = 4, got %0d", fres);
    else $display("PASSED");
  end

  function integer ufunc;
    input in;
    begin
      if (fres === 32'bx) fres = 0;
      if (in) fres = fres - 1;
      else fres = fres + 1;
      ufunc = fres;
    end
  endfunction
endmodule
