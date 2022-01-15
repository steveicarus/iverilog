module test;

  reg [2:0] ptr;
  reg [2:0] size;
  reg [2:0] ptr_nxt;

  always @*
    begin
      ptr_nxt = ptr;

      if ( ptr + size > 3 )
        begin
          ptr_nxt = ptr + size - 3;
        end
      else
        begin
          ptr_nxt = 0;
        end
    end

  initial
    begin
      #1;

      ptr = 2;
      size = 2;
      #1
      $write("ptr_nxt=%0d  ptr=%0d  size=%0d", ptr_nxt, ptr, size);
      if ( ptr_nxt == 1 )
        begin
          $display("  OK");
        end
      else
        begin
          $display("  ERROR");
	  $finish;
        end

      ptr = 3;
      size = 4;
      #1
      $write("ptr_nxt=%0d  ptr=%0d  size=%0d", ptr_nxt, ptr, size);
      if ( ptr_nxt == 4 )
        begin
          $display("  OK");
        end
      else
        begin
          $display("  ERROR");
	  $finish;
        end

      ptr = 3;
      size = 5;
      #1
      $write("ptr_nxt=%0d  ptr=%0d  size=%0d", ptr_nxt, ptr, size);
      if ( ptr_nxt == 5 )
        begin
          $display("  OK");
        end
      else
        begin
          $display("  ERROR");
	  $finish;
        end

      $display("PASSED");
      $finish;

    end

endmodule
