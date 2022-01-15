// Ten basic tests in here:
// 1. byte must be initialised before any initial or always block
// 2. assignments to (signed) bytes with random numbers
// 3. assignments to (signed) bytes with random values including X and Z
// 4. converting unsigned integers to signed bytes
// 5. converting signed integers to signed bytes
// 6. converting integers including X and Z states to signed bytes
// 7. trying signed sums (procedural, function, task and module)
// 8. trying signed mults (procedural, function and task)
// 9. trying relational operators
// 10. smaller signed numbers to signed bytes (sign extension)

module ms_add (input byte signed a, b, output byte signed sc, ss);
   assign sc = a + b;
   always @(a, b) ss = a + b;
endmodule

module main;
  parameter N_REPS = 500;               // repetition with random numbers
  parameter XZ_REPS = 500;              // repetition with 'x 'z values
  parameter MAX = 'h7f;
  parameter LEN = 8;
  // variables used as golden references
  reg signed [LEN-1:0] ar;              // holds numbers
  reg signed [LEN-1:0] ar_xz;           // holds 'x and/or 'z in random positions
  reg signed [LEN-1:0] ar_expected;
  integer unsigned       ui;
  integer signed         si;
  reg signed [LEN/2-1:0]       slice;
  // types to be tested
  byte signed bu;                       // holds numbers
  byte signed bu_xz;                    // 'x and 'z are attempted on this
  byte signed bresult;                  // hold results from sums and mults
  byte signed mcaresult;                // this is permanently wired to a module
  byte signed mabresult;                // this is permanently wired to a module

  integer i;

  // continuous assigments
  // type LHS      type RHS
  // ---------     ---------
  // byte          4-value logic
  assign bu = ar;
  assign bu_xz = ar_xz;
  // module instantiation

  ms_add duv (.a(bu), .b(bu_xz), .sc(mcaresult), .ss(mabresult) );

  // all test
  initial begin
    // time 0 checkings (Section 6.4 of IEEE 1850 LRM)
    if (bu !== 8'b0 || bu_xz != 8'b0 || bresult !== 8'b0)
      begin
        $display ("FAILED - time zero initialisation incorrect: %b %b", bu, bu_xz);
        $finish;
      end
    // driving byte type with signed random numbers from a variable
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = $random % MAX;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect assigment to byte: %b", bu);
            $finish;
        end
      end
    # 1;
    // attempting to drive variables having 'x 'z values into type unsigned bytes
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        ar = $random % MAX;
        ar_xz = xz_inject (ar);
        ar_expected = xz_expected (ar_xz);
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect assigment to byte (when 'x 'z): %b", bu);
            $finish;
        end
      end
    // converting unsigned integers to signed bytes
    // truncation expected (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ui = {$random} % 2*(MAX+1); // full range as unsigned
        #1;
        force bu = ui;
        #1;
        if (bu !== ui[LEN-1:0])
          begin
            $display ("FAILED - incorrect truncation from unsigned integer to byte: %b", bu);
            $finish;
        end
      end
    release bu;
    // converting signed integers to signed bytes
    // truncation expected (Section 4.3.2 of IEEE 1850 LRM)
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        si = $random % MAX;
        #1;
        force bu = si;
        #1;
        if (bu !== si[LEN-1:0])
          begin
            $display ("FAILED - incorrect truncation from signed integer to byte: %b mismatchs %b", bu, si[LEN-1:0]);
            $finish;
        end
      end
    release bu;
    // converting signed integers having 'x 'z values into type signed bytes
    // 'x 'z injections (Section 4.3.2 of IEEE 1850 LRM)
    // truncation and coercion to zero expected
    for (i = 0; i< XZ_REPS; i = i+1)
      begin
        #1;
        si = $random;
        ar_xz = xz_inject (si[LEN-1:0]);
        si = {si[31:LEN], ar_xz};
        ar_expected = xz_expected (ar_xz);
        #1;
        force bu_xz = si;
        #1;
        if (bu_xz !== ar_expected)       // 'x -> '0, 'z -> '0
          begin
            $display ("FAILED - incorrect conversion from integer (with 'x 'z) to byte: %b mismatchs %b", bu_xz, ar_expected);
            $finish;
        end
      end
    release bu_xz;
    // trying signed sums
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = $random % MAX;
        ar_xz = $random % MAX;
        #1;
        bresult = bu + bu_xz;
        #1;
        if ( bresult !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed bytes: %0d mismatchs %0d", bresult, s_sum(ar, ar_xz));
            $finish;
          end
        // invoking byte sum function
        if ( fs_sum (bu, bu_xz) !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed bytes in function");
            $finish;
          end
        // invoking byte sum task
        ts_sum (bu, bu_xz, bresult);
        if ( bresult !== s_sum(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect addition of signed bytes in task: %0d mismatchs %0d", bresult, s_sum(ar, ar_xz));
            $finish;
          end
        // checking byte sum from module
        if ( mcaresult !== s_sum(ar, ar_xz) || mabresult !== s_sum(ar, ar_xz))
          begin
            $display ("FAILED - incorrect addition of signed bytes from module");
            $finish;
          end
      end

    // trying signed mults, forcing truncation
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = ($random % MAX) << LEN/2;
        ar_xz = ($random % MAX) << (LEN/2 - 1);
        #1;
        bresult = bu * bu_xz;
        #1;
        if ( bresult !== s_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect product of signed bytes: %0d mismatchs %0d", bresult, s_mul(ar, ar_xz));
            $finish;
          end
        // invoking byte mult function
        if ( fs_mul (bu, bu_xz) !== s_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect product of signed bytes in function");
            $finish;
          end
        // invoking byte mult task
        ts_mul (bu, bu_xz, bresult);
        if ( bresult !== s_mul(ar, ar_xz) )
          begin
            $display ("FAILED - incorrect product of signed bytes in task: %0d mismatchs %0d", bresult, s_mul(ar, ar_xz));
            $finish;
          end
      end
    // trying relational operators
    for (i = 0; i< N_REPS; i = i+1)
      begin
        #1;
        ar = $random % MAX;
        ar_xz = $random % MAX;
        #1;
        if ( (bu < bu_xz ) !== (ar < ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than' on signed bytes");
            $finish;
          end
        if ( (bu <= bu_xz ) !== (ar <= ar_xz) )
          begin
            $display ("FAILED - incorrect 'less than or equal' on signed bytes");
            $finish;
          end
        if ( (bu > bu_xz ) !== (ar > ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than' on signed bytes");
            $finish;
          end
        if ( (bu >= bu_xz ) !== (ar >= ar_xz) )
          begin
            $display ("FAILED - incorrect 'greater than or equal' than on signed bytes");
            $finish;
          end
        if ( (bu == bu_xz ) !== (ar == ar_xz) )
          begin
            $display ("FAILED - incorrect 'equal to' on signed bytes");
            $finish;
          end
        if ( (bu != bu_xz ) !== (ar != ar_xz) )
          begin
            $display ("FAILED - incorrect 'not equal to' on signed bytes");
            $finish;
          end
      end
    // signed small number to signed byte
    for (i = 0; i < (1<<LEN/2); i = i+1)
      begin
        #1;
        slice = $random % 'h7;
        force bu = slice;
        ar = slice;
        #1;
        if (bu !== ar)
          begin
            $display ("FAILED - incorrect signed extend to signed bytes");
            $finish;
          end
      end
    release bu;
    # 1;
    $display("PASSED");
  end

  // this returns X and Z states into bit random positions for a value
  function [LEN-1:0] xz_inject (input signed [LEN-1:0] value);
      integer i, temp;
      begin
        temp = {$random} % 2*(MAX+1);  // possible 1 in any position
        for (i=0; i<LEN; i=i+1)
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
  function [LEN-1:0] xz_expected (input signed [LEN-1:0] value_xz);
      integer i;
      begin
        for (i=0; i<LEN; i=i+1)
          begin
             if (value_xz[i] === 1'bx || value_xz[i] === 1'bz )
                 value_xz[i] = 1'b0;  // forced to zero
          end
          xz_expected = value_xz;
      end
  endfunction

  // signed 4-value sum
  function signed [LEN-1:0] s_sum (input signed [LEN-1:0] a, b);
      s_sum = a + b;
  endfunction

   // signed byte sum as function
  function byte signed fs_sum (input byte signed a, b);
      fs_sum = a + b;
  endfunction

  // signed byte sum as task
  task ts_sum (input byte signed a, b, output byte signed c);
      c = a + b;
  endtask

   // signed 4-value mults
  function signed [LEN-1:0] s_mul (input signed [LEN-1:0] a, b);
      s_mul = a * b;
  endfunction

  // signed byte mults
  function byte signed fs_mul (input byte signed a, b);
      fs_mul = a * b;
  endfunction

  // signed byte mult as task
  task ts_mul (input byte signed a, b, output byte signed c);
      c = a * b;
  endtask


endmodule
