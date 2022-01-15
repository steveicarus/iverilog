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
// Synchronous FIFO.  4 x 16 bit words.
//
// Modified by SDW to print out PASSED only if DEBUG not defined.
// Also changed TEST1 so that it is "self checking" by adding a
// passed in value to read_word.
//
module fifo (clk, rstp, din, writep, readp, dout, emptyp, fullp);
input		clk;
input		rstp;
input [15:0]	din;
input		readp;
input		writep;
output [15:0]	dout;
output		emptyp;
output		fullp;

// Defines sizes in terms of bits.
//
parameter	DEPTH = 3,		// 2 bits, e.g. 4 words in the FIFO.
		MAX_COUNT = 3'b111;	// topmost address in FIFO.

reg		emptyp;
reg		fullp;

// Registered output.
reg [15:0]	dout;

// Define the FIFO pointers.  A FIFO is essentially a circular queue.
//
reg [(DEPTH-1):0]	tail;
reg [(DEPTH-1):0]	head;

// Define the FIFO counter.  Counts the number of entries in the FIFO which
// is how we figure out things like Empty and Full.
//
reg [(DEPTH-1):0]	count;

// Define our regsiter bank.  This is actually synthesizable!
//
reg [15:0] fifomem[0:MAX_COUNT];

// Dout is registered and gets the value that tail points to RIGHT NOW.
//
always @(posedge clk)
 begin
   if (rstp == 1)
      dout <= 16'h0000;
   else
      dout <= fifomem[tail];
 end


// Update FIFO memory.
always @(posedge clk) begin
   if (rstp == 1'b0 && writep == 1'b1 && fullp == 1'b0) begin
      fifomem[head] <= din;
   end
end

// Update the head register.
//
always @(posedge clk) begin
   if (rstp == 1'b1) begin
      head <= 2'b00;
   end
   else begin
      if (writep == 1'b1 && fullp == 1'b0) begin
         // WRITE
         head <= head + 1;
      end
   end
end

// Update the tail register.
//
always @(posedge clk) begin
   if (rstp == 1'b1) begin
      tail <= 2'b00;
   end
   else begin
      if (readp == 1'b1 && emptyp == 1'b0) begin
         // READ
         tail <= tail + 1;
      end
   end
end

// Update the count regsiter.
//
always @(posedge clk) begin
   if (rstp == 1'b1) begin
      count <= 2'b00;
   end
   else begin
      case ({readp, writep})
         2'b00: count <= count;
         2'b01:
            // WRITE
            if (count != MAX_COUNT)
               count <= count + 1;
         2'b10:
            // READ
            if (count != 2'b00)
               count <= count - 1;
         2'b11:
            // Concurrent read and write.. no change in count
            count <= count;
      endcase
   end
end


// *** Update the flags
//
// First, update the empty flag.
//
always @(count) begin
   if (count == 2'b00)
      emptyp <= 1'b1;
   else
      emptyp <= 1'b0;
end


// Update the full flag
//
always @(count) begin
   if (count == MAX_COUNT)
      fullp <= 1'b1;
   else
      fullp <= 1'b0;
end

endmodule

// synopsys translate_off

`define TEST_FIFO
// synopsys translate_off
`ifdef TEST_FIFO


module test_fifo;

reg		clk;
reg		rstp;
reg [15:0]	din;
reg		readp;
reg		writep;
wire [15:0]	dout;
wire		emptyp;
wire		fullp;
reg		error ;

reg [15:0]	value;

fifo U1 (
   .clk		(clk),
   .rstp	(rstp),
   .din		(din),
   .readp	(readp),
   .writep	(writep),
   .dout	(dout),
   .emptyp	(emptyp),
   .fullp	(fullp)
);

//
// SDW Added self testing aspect here..
//
task read_word;
input [15:0] expect;
begin
   @(negedge clk);
   readp = 1;
   @(posedge clk) #5;
`ifdef DEBUG
   $display ("Expect %0h, Read %0h from FIFO",
`endif // DEBUG
   if(expect !== dout)
     begin
        $display ("FAILED - Expect %0h, Read %0h from FIFO",
                       expect,dout);
        error = 1;
     end
   readp = 0;
end
endtask

task write_word;
input [15:0]	value;
begin
   @(negedge clk);
   din = value;
   writep = 1;
   @(posedge clk);
`ifdef DEBUG
   $display ("Write %0h to FIFO", din);
`endif // DEBUG
   #5;
   din = 16'hzzzz;
   writep = 0;
end
endtask

initial begin
   clk = 0;
   forever begin
      #10 clk = 1;
      #10 clk = 0;
   end
end

initial begin
   error = 0;	// Set error to zero here.
`ifdef DEBUG
   $dumpfile("test.vcd");
   $dumpvars(0,test_fifo);
`endif // DEBUG
   test1;
   //test2;

   if(error == 0)
      $display("PASSED");
   $finish;
end

task test1;
begin
   din = 16'hzzzz;
   writep = 0;
   readp = 0;

   // Reset
   rstp = 1;
   #50;
   rstp = 0;
   #50;

   // ** Write 3 values.
   write_word (16'h1111);
   write_word (16'h2222);
   write_word (16'h3333);

   // ** Read 2 values
   read_word(16'h1111);
   read_word(16'h2222);

   // ** Write one more
   write_word (16'h4444);

   // ** Read a bunch of values
   read_word(16'h3333);

   // *** Write a bunch more values
   write_word (16'h0001);
   write_word (16'h0002);
   write_word (16'h0003);
   write_word (16'h0004);
   write_word (16'h0005);
   write_word (16'h0006);
   write_word (16'h0007);
   write_word (16'h0008);

   // ** Read a bunch of values
   read_word(16'h4444);
   read_word(16'h0001);
   read_word(16'h0002);
   read_word(16'h0003);
   read_word(16'h0004);
   read_word(16'h0005);
   read_word(16'h0006);
end
endtask
`ifdef TEST2
// TEST2
//
// This test will operate the FIFO in an orderly manner the way it normally works.
// 2 threads are forked; a reader and a writer.  The writer writes a counter to
// the FIFO and obeys the fullp flag and delays randomly.  The reader likewise
// obeys the emptyp flag and reads at random intervals.  The result should be that
// the reader reads the incrementing counter out of the FIFO.  The empty/full flags
// should bounce around depending on the random delays.  The writer repeats some
// fixed number of times and then terminates both threads and kills the sim.
//
task test2;
reg [15:0] writer_counter;
begin
   writer_counter = 16'h0001;
   din = 16'hzzzz;
   writep = 0;
   readp = 0;

   // Reset
   rstp = 1;
   #50;
   rstp = 0;
   #50;

   fork
      // Writer
      begin
         repeat (500) begin
            @(negedge clk);
            if (fullp == 1'b0) begin
               write_word (writer_counter);
               #5;
               writer_counter = writer_counter + 1;
            end
            else begin
               $display ("WRITER is waiting..");
            end
            // Delay a random amount of time between 0ns and 100ns
            #22 ;
         end
         $display ("Done with WRITER fork..");
         $finish;
      end

      // Reader
      begin
         forever begin
            @(negedge clk);
            if (emptyp == 1'b0) begin
               read_word;
            end
            else begin
               $display ("READER is waiting..");
            end
            // Delay a random amount of time between 0ns and 100ns
            #50;
         end
      end
   join
end
endtask

/*
always @(fullp)
   $display ("fullp = %0b", fullp);

always @(emptyp)
   $display ("emptyp = %0b", emptyp);

always @(U1.head)
   $display ("head = %0h", U1.head);

always @(U1.tail)
   $display ("tail = %0h", U1.tail);
*/

`endif // TEST2

endmodule
`endif
`end_keywords
