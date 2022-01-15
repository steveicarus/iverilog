// Three basic tests in here:
// 1. bit must be initialised before any initial or always block
// 2. assignments to (unsigned) bits with random numbers
// 3. assignments to (unsigned) bits with random values including X and Z


module ibit_test;
  parameter TRIALS = 100;
  parameter MAX = 32768;

  reg unsigned [14:0] ar;              // should it be "reg unsigned [7:0] aw"?
  reg unsigned [14:0] ar_xz;           // same as above here?
  reg unsigned [14:0] ar_expected;     // and here
  bit unsigned [14:0] bu;
  bit unsigned [14:0] bu_xz;

  integer i;

  assign bu = ar;
  assign bu_xz = ar_xz;

  // all test
  initial begin
    // time 0 checkings (Section 6.4 of IEEE 1850 LRM)
    if (bu !== 15'b0 | bu_xz != 15'b0)
      begin
        $display ("FAILED - time zero initialisation incorrect: %b %b", bu, bu_xz);
        $finish;
      end
    // random numbers
    for (i = 0; i< TRIALS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect assigment to bits: %b", bu);
            $finish;
        end
      end
    # 1;
    // with 'x injections (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< TRIALS; i = i+1)
      begin
        #1;
        ar = {$random} % MAX;
        ar_xz = xz_inject (ar);
        ar_expected = xz_expected (ar_xz);
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect assigment to bits (when 'x): %b", bu);
            $finish;
        end
      end
    # 1;
    $display("PASSED");
  end

  // this returns X and Z states into bit random positions for a value
  function [15:0] xz_inject (input reg unsigned [15:0] value); // should it be "input unsigned [15:0]" instead?
      integer i, temp;
      begin
        temp = {$random} % MAX;
        for (i=0; i<15; i=i+1)
          begin
             if (temp[i] == 1'b1)
               begin
                 temp = $random % MAX;
                 if (temp <= 0)
                    value[i] = 1'bx;  // 'x noise
                 else
                    value[i] = 1'bz;  // 'z noise
               end
          end
          xz_inject = value;
      end
  endfunction

  // this function returns bit positions with either X or Z to 0 for an input value
  function [15:0] xz_expected (input reg unsigned [15:0] value_xz); // should it be "input unsigned [15:0] instead?
      integer i;
      begin
        for (i=0; i<15; i=i+1)
          begin
             if (value_xz[i] === 1'bx || value_xz[i] === 1'bz )
                 value_xz[i] = 1'b0;  // forced to zero
          end
          xz_expected = value_xz;
      end
  endfunction


endmodule
