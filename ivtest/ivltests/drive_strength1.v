/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/*
 * This program tests some tricky-to-compile strength syntax. Show that
 * gates can drive a wire with various strengths and be properly resolved.
 */
module main();

   reg pullval;
   wire (weak0, weak1) value = pullval;

   reg	en0, en1;

   /* This buffer will drive a strong 1 to value if en0 is 1, otherwise
      it will go HiZ. */
   buf (highz0, strong1) drive0(value, en0);

   /* This inverter will drive a strong 0 to value if en1 is 1, otherwise
      is will go HiZ. */
   not (strong0, highz1) drive1(value, en1);


   initial begin
      en0 = 0;
      en1 = 0;

      /* Make sure when the other drivers are disabled, the pullval
         can pull the value up or down. The gates should be HiZ. */
      pullval = 1;
      #1 if (value !== 1'b1) begin
	 $display("FAILED -- value is %b", value);
	 $finish;
      end

      pullval = 0;
      #1 if (value !== 1'b0) begin
	 $display("FAILED -- value is %b", value);
	 $finish;
      end

      /* When en0 is 1, drive0 puts a strong 1 onto value so the
         pullval should not matter. */
      en0 = 1;
      pullval = 1;
      #1 if (value !== 1'b1) begin
	 $display("FAILED -- en0==%b en1==%b pull==%b value==%b",
		  en0, en1, pullval, value);
	 $finish;
      end

      pullval = 0;
      #1 if (value !== 1'b1) begin
	 $display("FAILED -- en0==%b en1==%b pull=0%b value==%b",
		  en0, en1, pullval, value);
	 $finish;
      end

      /* When en1 is 1, drive1 puts a strong 0 onto value so the
         pullval should not matter. */
      en0 = 0;
      en1 = 1;
      pullval = 1;
      #1 if (value !== 1'b0) begin
	 $display("FAILED -- en0==%b en1==%b pull=0%b value==%b",
		  en0, en1, pullval, value);
	 $finish;
      end

      pullval = 0;
      #1 if (value !== 1'b0) begin
	 $display("FAILED -- en0==%b en1==%b pull=0%b value==%b",
		  en0, en1, pullval, value);
	 $finish;
      end

      /* When both enables are 1, we have a double driven signal
         and the value should be x. */
      en0 = 1;
      en1 = 1;
      pullval = 1;
      #1 if (value !== 1'bx) begin
	 $display("FAILED -- en0==%b en1==%b pull=0%b value==%b",
		  en0, en1, pullval, value);
	 $finish;
      end

      pullval = 0;
      #1 if (value !== 1'bx) begin
	 $display("FAILED -- en0==%b en1==%b pull=0%b value==%b",
		  en0, en1, pullval, value);
	 $finish;
      end


      $display("PASSED");
   end // initial begin

endmodule // main
