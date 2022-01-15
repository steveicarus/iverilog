`celldefine
`suppress_faults
`enable_portfaults





   `timescale 1ns / 10ps
   `delay_mode_path





module test (Z, A, B);

   output Z;
   input A;
   input B;

   xor (Z, A, B);



   specify

   ifnone (A +=> Z) = (1.0,1.0);

   endspecify


endmodule
`disable_portfaults
`nosuppress_faults
`endcelldefine
