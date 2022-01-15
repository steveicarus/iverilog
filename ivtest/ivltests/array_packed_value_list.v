// This tests assigning value lists to packed arrays
//
// This file ONLY is placed into the Public Domain, for any use,
// without warranty, 2012 by Iztok Jeras.

module test ();

   // parameters for array sizes
   localparam WA = 4;
   localparam WB = 4;

   // 2D packed arrays
   logic [WA-1:0] [WB-1:0] abg0, abg1, abg2, abg3, abg4, abg5, abg6, abg7, abg8, abg9;  // big    endian array
   logic [0:WA-1] [0:WB-1] alt0, alt1, alt2, alt3, alt4, alt5, alt6, alt7, alt8, alt9;  // little endian array

   // error counter
   bit err = 0;

   initial begin
      abg0               = '{ 3 ,2 ,1, 0 };
      abg1               = '{0:4, 1:5, 2:6, 3:7};
      abg2               = '{default:13};
      abg3               = '{2:15, default:13};
      abg4               = '{WA  {          {WB/2  {2'b10}}  }};
      abg5               = '{WA  { {3'b101, {WB/2-1{2'b10}}} }};
      abg6               = '{WA  {          {WB/2-1{2'b10}}  }};
      abg7 [WA/2-1:0   ] = '{WA/2{          {WB/2  {2'b10}}  }};
      abg8 [WA  -1:WA/2] = '{WA/2{          {WB/2  {2'b01}}  }};
      abg9               = '{err+0, err+1, err+2, err+3};
      // check
      if (abg0 !== 16'b0011_0010_0001_0000) begin $display("FAILED -- abg0 = 'b%b", abg0); err=1; end
      if (abg1 !== 16'b0111_0110_0101_0100) begin $display("FAILED -- abg1 = 'b%b", abg1); err=1; end
      if (abg2 !== 16'b1101_1101_1101_1101) begin $display("FAILED -- abg2 = 'b%b", abg2); err=1; end
      if (abg3 !== 16'b1101_1111_1101_1101) begin $display("FAILED -- abg3 = 'b%b", abg3); err=1; end
      if (abg4 !== 16'b1010_1010_1010_1010) begin $display("FAILED -- abg4 = 'b%b", abg4); err=1; end
      if (abg5 !== 16'b0110_0110_0110_0110) begin $display("FAILED -- abg5 = 'b%b", abg5); err=1; end
      if (abg6 !== 16'b0010_0010_0010_0010) begin $display("FAILED -- abg6 = 'b%b", abg6); err=1; end
      if (abg7 !== 16'bxxxx_xxxx_1010_1010) begin $display("FAILED -- abg7 = 'b%b", abg7); err=1; end
      if (abg8 !== 16'b1010_1010_xxxx_xxxx) begin $display("FAILED -- abg8 = 'b%b", abg8); err=1; end
      if (abg9 !== 16'b0000_0001_0010_0011) begin $display("FAILED -- abg9 = 'b%b", abg9); err=1; end

      alt0               = '{ 3 ,2 ,1, 0 };
      alt1               = '{0:4, 1:5, 2:6, 3:7};
      alt2               = '{default:13};
      alt3               = '{2:15, default:13};
      alt4               = '{WA  {          {WB/2  {2'b10}}  }};
      alt5               = '{WA  { {3'b101, {WB/2-1{2'b10}}} }};
      alt6               = '{WA  {          {WB/2-1{2'b10}}  }};
      alt7 [0   :WA/2-1] = '{WA/2{          {WB/2  {2'b10}}  }};
      alt8 [WA/2:WA  -1] = '{WA/2{          {WB/2  {2'b01}}  }};
      alt9               = '{err+0, err+1, err+2, err+3};
      // check
      if (alt0 !== 16'b0011_0010_0001_0000) begin $display("FAILED -- alt0 = 'b%b", alt0); err=1; end
      if (alt1 !== 16'b0100_0101_0110_0111) begin $display("FAILED -- alt1 = 'b%b", alt1); err=1; end
      if (alt2 !== 16'b1101_1101_1101_1101) begin $display("FAILED -- alt2 = 'b%b", alt2); err=1; end
      if (alt3 !== 16'b1101_1101_1111_1101) begin $display("FAILED -- alt3 = 'b%b", alt3); err=1; end
      if (alt4 !== 16'b1010_1010_1010_1010) begin $display("FAILED -- alt4 = 'b%b", alt4); err=1; end
      if (alt5 !== 16'b0110_0110_0110_0110) begin $display("FAILED -- alt5 = 'b%b", alt5); err=1; end
      if (alt6 !== 16'b0010_0010_0010_0010) begin $display("FAILED -- alt6 = 'b%b", alt6); err=1; end
      if (alt7 !== 16'b1010_1010_xxxx_xxxx) begin $display("FAILED -- alt7 = 'b%b", alt7); err=1; end
      if (alt8 !== 16'bxxxx_xxxx_1010_1010) begin $display("FAILED -- alt8 = 'b%b", alt8); err=1; end
      if (alt9 !== 16'b0000_0001_0010_0011) begin $display("FAILED -- alt9 = 'b%b", alt9); err=1; end

      if (!err) $display("PASSED");
   end

endmodule // test
