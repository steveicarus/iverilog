`timescale 100 ps / 10 ps

module test;
  reg cdn, cp, d;
  wire qr, qp;

  initial begin
    $timeformat(-9, 2, " ns", 9);
    $monitor("%t - cdn=%b, cp=%b, d=%b, qr=%b, qp=%b",$time,cdn,cp,d,qr,qp);
    // Reset the FF.
    cp = 0; d = 1;
    #100 cdn = 0;
    #100 cdn = 1;
    // Toggle in some data.
    #100 cp = 1;
    #100 cp = 0; d = 0;
    #100 cp = 1;
    #100 cp = 0; d = 1;
    #100 cp = 1;
  end

  dff_rtl dutr(qr, d, cp, cdn);  // This one works fine.
  dff_prm dutp(qp, d, cp, cdn);  // This one has no delay.
endmodule

// The RTL version appears to work fine.
module dff_rtl (q, d, cp, cdn);
  output q;
  input d;
  input cp;
  input cdn;

  reg qi;

  always @(posedge cp or negedge cdn) begin
    if (~cdn) qi <= 1'b0;
    else qi <= d;
  end
  buf (q, qi);

  specify
    specparam tpd_cp_q_lh = 6;
    specparam tpd_cp_q_hl = 7;
    specparam tpd_cdn_q_lh = 0;
    specparam tpd_cdn_q_hl = 3;

    if (cdn) (posedge cp => (q +: d)) = (tpd_cp_q_lh, tpd_cp_q_hl);
    (negedge cdn => (q +: 1'b0)) = (tpd_cdn_q_lh, tpd_cdn_q_hl);
  endspecify
endmodule

// The primitive version has no delay.
module dff_prm (q, d, cp, cdn);
  output q;
  input d, cp, cdn;

  UDP_DFF G3(q, d, cp, cdn);

  specify
    specparam tpd_cp_q_lh = 6;
    specparam tpd_cp_q_hl = 7;
    specparam tpd_cdn_q_lh = 0;
    specparam tpd_cdn_q_hl = 3;

    if (cdn) (posedge cp => (q +: d)) = (tpd_cp_q_lh, tpd_cp_q_hl);
    (negedge cdn => (q +: 1'b0)) = (tpd_cdn_q_lh, tpd_cdn_q_hl);
  endspecify

endmodule

// This is overly simplistic, but it works for this example.
primitive UDP_DFF(q, d, cp, cdn);
  output q;
  reg q;
  input d, cp, cdn;

  table
  // d  cp  cdn   q0   q
     *   ?    ?  : ? : - ;
     ?   n    ?  : ? : - ;
     0   r    1  : ? : 0 ;
     1   r    1  : ? : 1 ;
     ?   ?    0  : ? : 0 ;
     ?   ?    p  : ? : - ;
  endtable
endprimitive
