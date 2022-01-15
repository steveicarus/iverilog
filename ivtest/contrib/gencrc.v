`begin_keywords "1364-2005"
//
// Copyright (c) 1999 Thomas Coonan (tcoonan@mindspring.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//
//
// Behavioral Verilog for CRC16 and CRC32 for use in a testbench.
//
// The specific polynomials and conventions regarding bit-ordering etc.
// are specific to the Cable Modem DOCSIS protocol, but the general scheme
// should be reusable for other types of CRCs with some fiddling.
//
// This CRC code works for a specific type of network protocol, and it
// must do certain byte swappings, etc.  You may need to play with it
// for your protocol.  Also, make sure the polynomials are what you
// really want.  This is obviously, not synthesizable - I just used this
// in a testbench at one point.
//
// These tasks are crude and rely on some global parameters.  They should
// also read from a file, yada yada yada.  It is probably better to do this
// with a PLI call, but here it is anyway..
//
// The test case includes a golden DOCSIS (Cable Modem) test message that
// was captured in a lab.
//
// tom coonan, 1999.
//
module test_gencrc;

// *** Buffer for the Golden Message ***
reg [7:0]	test_packet[0:54];

// *** Global parameter block for the CRC32 calculator.
//
parameter	CRC32_POLY = 32'h04C11DB7;
reg [ 7:0]	crc32_packet[0:255];
integer		crc32_length;
reg [31:0]	crc32_result;

// *** Global parameter block for the CRC16 calculator.
//
parameter	CRC16_POLY = 16'h1020;
reg [ 7:0]	crc16_packet[0:255];
integer		crc16_length;
reg [15:0]	crc16_result;

`define TEST_GENCRC
`ifdef TEST_GENCRC
// Call the main test task and then quit.
//
initial begin
   main_test;
   $finish;
end
`endif

// ****************************************************************
// *
// *   GOLDEN MESSAGE
// *
// *   The golden message is a DOCSIS frame that was captured off
// *   the Broadcom reference design.  It is a MAP message.  It
// *   includes a HCS (crc 16) and a CRC32.
// *
// *
// ****************************************************************
//
task initialize_test_packet;
   begin
      test_packet[00] = 8'hC2;	// FC.   HCS coverage starts here.
      test_packet[01] = 8'h00;	// MACPARAM
      test_packet[02] = 8'h00;	// MAC LEN
      test_packet[03] = 8'h30;	// MAC LEN.  HCS Coverage includes this byte and ends here.
      test_packet[04] = 8'hF2;	// CRC16 (also known as HCS)
      test_packet[05] = 8'hCF;	// CRC16 cont..
      test_packet[06] = 8'h01;	// Start of the IEEE payload.  CRC32 covererage starts here.  This is the DA field
      test_packet[07] = 8'hE0;	// DA field cont..
      test_packet[08] = 8'h2F;	// DA field cont..
      test_packet[09] = 8'h00;	// DA field cont..
      test_packet[10] = 8'h00;	// DA field cont..
      test_packet[11] = 8'h01;	// DA field cont..
      test_packet[12] = 8'h00;	// SA field
      test_packet[13] = 8'h80;	// SA field cont..
      test_packet[14] = 8'h42;	// SA field cont..
      test_packet[15] = 8'h42;	// SA field cont..
      test_packet[16] = 8'h20;	// SA field cont..
      test_packet[17] = 8'h9E;	// SA field cont..
      test_packet[18] = 8'h00;	// IEEE LEN field
      test_packet[19] = 8'h1E;	// IEEE LEN field cont.
      test_packet[20] = 8'h00;	// LLC field.
      test_packet[21] = 8'h00;	// LLC field cont...
      test_packet[22] = 8'h03;	// LLC field cont...
      test_packet[23] = 8'h01;	// LLC field cont...
      test_packet[24] = 8'h03;	// LLC field cont...  This is also the TYPE, which indicates MAP.
      test_packet[25] = 8'h00;	// LLC field cont...
      test_packet[26] = 8'h01;	// Start of MAP message payload.
      test_packet[27] = 8'h01;	// MAP message payload..
      test_packet[28] = 8'h02;	// MAP message payload..
      test_packet[29] = 8'h00;	// MAP message payload..
      test_packet[30] = 8'h00;	// MAP message payload..
      test_packet[31] = 8'h18;	// MAP message payload..
      test_packet[32] = 8'hAA;	// MAP message payload..
      test_packet[33] = 8'h58;	// MAP message payload..
      test_packet[34] = 8'h00;	// MAP message payload..
      test_packet[35] = 8'h18;	// MAP message payload..
      test_packet[36] = 8'hA8;	// MAP message payload..
      test_packet[37] = 8'hA0;	// MAP message payload..
      test_packet[38] = 8'h02;	// MAP message payload..
      test_packet[39] = 8'h03;	// MAP message payload..
      test_packet[40] = 8'h03;	// MAP message payload..
      test_packet[41] = 8'h08;	// MAP message payload..
      test_packet[42] = 8'hFF;	// MAP message payload..
      test_packet[43] = 8'hFC;	// MAP message payload..
      test_packet[44] = 8'h40;	// MAP message payload..
      test_packet[45] = 8'h00;	// MAP message payload..
      test_packet[46] = 8'h00;	// MAP message payload..
      test_packet[47] = 8'h01;	// MAP message payload..
      test_packet[48] = 8'hC0;	// MAP message payload..
      test_packet[49] = 8'h14;	// Last byte of MAP payload, last byte covered by CRC32.
      test_packet[50] = 8'hDD;	// CRC32 Starts here
      test_packet[51] = 8'hBF;	// CRC32 cont..
      test_packet[52] = 8'hC1;	// CRC32 cont..
      test_packet[53] = 8'h2E;	// Last byte of CRC32, last byte of DOCSIS.
   end
endtask

// *************************************************************************
// *
// *   Main test task.
// *
// *   Use our primary "golden packet".  Copy into the generic global
// *   variables that the low-level 'gencrc16' and 'gencrc32' tasks use.
// *   Comare against the expected values and report SUCCESS or FAILURE.
// *
// *************************************************************************
//
task main_test;
   integer	i, j;
   integer	num_errors;
   reg [15:0]	crc16_expected;
   reg [31:0]	crc32_expected;
   begin

   num_errors = 0;

   // Initialize the Golden Message!
   //
   initialize_test_packet;

   // **** TEST CRC16
   //
   //
   // Copy golden test_packet into the main crc16 buffer..
   for (i=0; i<4; i=i+1) begin
      crc16_packet[i] = test_packet[i];
   end
   crc16_expected = {test_packet[4], test_packet[5]};
   crc16_length = 4;  // Must tell test function the length
   gencrc16;  // Call main test function
   if (crc16_result !== crc16_expected)
   begin
      num_errors = num_errors + 1;
      $display ("FAILED - Actual crc16_result = %h, Expected = %h",
      crc16_result, crc16_expected);
   end

   // **** TEST CRC16
   //
   j = 0;
   for (i=6; i<50; i=i+1) begin
      crc32_packet[j] = test_packet[i];
      j = j + 1;
   end
   crc32_expected = {test_packet[50], test_packet[51], test_packet[52], test_packet[53]};
   crc32_length = 44;
   gencrc32;
   if (crc32_result !== crc32_expected)
     begin
       $display ("FAILED - Actual crc32_result = %h, Expected = %h",
       crc32_result, crc32_expected);
       num_errors = num_errors + 1;
     end

   if(num_errors == 0)
    $display("PASSED");
end

endtask


// ****************************************************************
// *
// *   Main working CRC tasks are: gencrc16, gencrc32.
// *
// *   These tasks rely on some globals (see front of program).
// *
// ****************************************************************


// Generate a (DOCSIS) CRC16.
//
// Uses the GLOBAL variables:
//
//    Globals referenced:
//       parameter	CRC16_POLY = 16'h1020;
//       reg [ 7:0]	crc16_packet[0:255];
//       integer	crc16_length;
//
//    Globals modified:
//       reg [15:0]	crc16_result;
//
task gencrc16;
   integer	byte, bit;
   reg		msb;
   reg [7:0]	current_byte;
   reg [15:0]	temp;
   begin
      crc16_result = 16'hffff;
      for (byte = 0; byte < crc16_length; byte = byte + 1) begin
         current_byte = crc16_packet[byte];
         for (bit = 0; bit < 8; bit = bit + 1) begin
            msb = crc16_result[15];
            crc16_result = crc16_result << 1;
            if (msb != current_byte[bit]) begin
               crc16_result = crc16_result ^ CRC16_POLY;
               crc16_result[0] = 1;
            end
         end
      end

      // Last step is to "mirror" every bit, swap the 2 bytes, and then complement each bit.
      //
      // Mirror:
      for (bit = 0; bit < 16; bit = bit + 1)
         temp[15-bit] = crc16_result[bit];

      // Swap and Complement:
      crc16_result = ~{temp[7:0], temp[15:8]};
   end
endtask


// Generate a (DOCSIS) CRC32.
//
// Uses the GLOBAL variables:
//
//    Globals referenced:
//       parameter	CRC32_POLY = 32'h04C11DB7;
//       reg [ 7:0]	crc32_packet[0:255];
//       integer	crc32_length;
//
//    Globals modified:
//       reg [31:0]	crc32_result;
//

task gencrc32;
   integer	byte, bit;
   reg		msb;
   reg [7:0]	current_byte;
   reg [31:0]	temp;
   begin
      crc32_result = 32'hffffffff;
      for (byte = 0; byte < crc32_length; byte = byte + 1) begin
         current_byte = crc32_packet[byte];
         for (bit = 0; bit < 8; bit = bit + 1) begin
            msb = crc32_result[31];
            crc32_result = crc32_result << 1;
            if (msb != current_byte[bit]) begin
               crc32_result = crc32_result ^ CRC32_POLY;
               crc32_result[0] = 1;
            end
         end
      end

      // Last step is to "mirror" every bit, swap the 4 bytes, and then complement each bit.
      //
      // Mirror:
      for (bit = 0; bit < 32; bit = bit + 1)
         temp[31-bit] = crc32_result[bit];

      // Swap and Complement:
      crc32_result = ~{temp[7:0], temp[15:8], temp[23:16], temp[31:24]};
   end
endtask

endmodule
`end_keywords
