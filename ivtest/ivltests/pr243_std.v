/*
 * Copyright (c) 2001 Uwe Bonnes
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
`define ADC_DATA_OFFSET 5
`define ADC_CHANELS 8*48
//`define ADC_CHANELS 348

module mymod (out1,out2,state,reset);

   input [8:0] state;
   input       reset;
   output      out1,out2;
   assign out1 = (state > `ADC_DATA_OFFSET) ? 1 : 0;
   assign out2 = (state > `ADC_CHANELS + `ADC_DATA_OFFSET +1)|| (reset);

endmodule // mymod

module t;
   reg [8:0] state;
   reg       reset;
   wire      out1,out2;

   mymod m1 (out1,out2,state,reset);

   initial
     begin
        //$timeformat(-9,0,"ns",5);
        $display("                TIME:state:out1:out2");
        $monitor("%t:%5d:%4d:%4d",$time,state,out1,out2);
        state =0;
        reset = 0;
        #10
          reset=1;
        #20
          reset=0;
        #5110
          $finish;
     end
   always
     begin
        #10
          if (reset)
            state = 0;
          else
            state=state+1;
     end
endmodule // t
