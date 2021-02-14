void dlsch_rx_stbc(LTE_DL_FRAME_PARMS *frame_parms,
                          int ** rxdataF_ext,
                          int ** rxdataF_comp,
                          int **dl_ch_ext,
                          unsigned char symbol,
                          unsigned short nb_rb,
                          int16_t ampShift,
                          uint8_t Qm) {

  // NEED HARDCODED QAM TABLE FOR 4-QAM, 16-QAM, and 64-QAM.

  int16_t QAM_2[8] = {7071, 7071, 7071, -7071, -7071, 7071, -7071, -7071};
  
  // int16_t QAM_2[8] = {-23170 ,23170 ,-23170 ,-23170 ,23170 ,23170 ,23170 ,-23170};
  int16_t QAM_4[32] = {-31086,31086,-31086,10362,-31086,-31086,-31086,-10362,-10362,31086,-10362,10362,-10362,-31086,-10362,-10362,31086,31086,31086,10362,31086,-31086,31086,-10362,10362,31086,10362,10362,10362,-31086,10362,-10362};
  int16_t QAM_6[128] = {-35393,35393,-35393,25281,-35393,5056,-35393,15168,-35393,-35393,-35393,-25281,-35393,-5056,-35393,-15168,-25281,35393,-25281,25281,-25281,5056,-25281,15168,-25281,-35393,-25281,-25281,-25281,-5056,-25281,-15168,-5056,35393,-5056,25281,-5056,5056,-5056,15168,-5056,-35393,-5056,-25281,-5056,-5056,-5056,-15168,-15168,35393,-15168,25281,-15168,5056,-15168,15168,-15168,-35393,-15168,-25281,-15168,-5056,-15168,-15168,35393,35393,35393,25281,35393,5056,35393,15168,35393,-35393,35393,-25281,35393,-5056,35393,-15168,25281,35393,25281,25281,25281,5056,25281,15168,25281,-35393,25281,-25281,25281,-5056,25281,-15168,5056,35393,5056,25281,5056,5056,5056,15168,5056,-35393,5056,-25281,5056,-5056,5056,-15168,15168,35393,15168,25281,15168,5056,15168,15168,15168,-35393,15168,-25281,15168,-5056,15168,-15168};

  int16_t *rxF0,*rxF1;
  int16_t * QAM_TABLE;
  int qam_pt[2] = {0, 0};
  int16_t *ch_11,*ch_12,*ch_21,*ch_22;
  int32_t ch11p, ch12p, ch21p, ch22p, chAvg2, chAvg;
  // int16_t *z1, *z11, *z11_tmp, *z12, *z12_tmp, *z2, *z21, *z21_tmp, *z22, *z22_tmp, *z3, *z31, *z31_tmp, *z32, *z32_tmp, *z4, *z41, *z41_tmp, *z42, *z42_tmp;
  uint64_t ** MSE;
  int32_t chPwr = 0, symAmp;
  int iter1, iter2,x=0,y=0;
  uint64_t min;
  unsigned char rb,re;
  int jj = (symbol*frame_parms->N_RB_DL*12);
  uint8_t symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  uint8_t pilots = ((symbol_mod==0)||(symbol_mod==(4-frame_parms->Ncp))) ? 1 : 0;

  if (symbol >= 7) {
    return;
  }

  int16_t * rxF0_comp = (int16_t *) &rxdataF_comp[0][2 * jj];

  // STC - Input Symbols
  int32_t * s1 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * s2 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * s3 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * s4 = (int32_t *)calloc(2, sizeof(int32_t));

  int32_t * z1 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z11 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z11_tmp = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z12 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z12_tmp = (int32_t *)calloc(2, sizeof(int32_t));
  
  int32_t * z2 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z21 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z21_tmp = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z22 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z22_tmp = (int32_t *)calloc(2, sizeof(int32_t));
  
  int32_t * z3 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z31 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z31_tmp = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z32 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z32_tmp = (int32_t *)calloc(2, sizeof(int32_t));

  int32_t * z4 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z41 = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z41_tmp = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * z42 = (int32_t *)calloc(2, sizeof(int32_t));  
  int32_t * z42_tmp = (int32_t *)calloc(2, sizeof(int32_t));
          
  // STC - Coding Parameters
  int32_t * a = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * b = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * c = (int32_t *)calloc(2, sizeof(int32_t));
  int32_t * d = (int32_t *)calloc(2, sizeof(int32_t));

  rxF0     = (int16_t *)&rxdataF_ext[0][jj];
  rxF1     = (int16_t *)&rxdataF_ext[1][jj];

  ch_11 = (int16_t *)&dl_ch_ext[0][jj];
  ch_21 = (int16_t *)&dl_ch_ext[1][jj];
  ch_12 = (int16_t *)&dl_ch_ext[2][jj];
  ch_22 = (int16_t *)&dl_ch_ext[3][jj];

  ch11p = ((int32_t)ch_11[0] * ch_11[0] + (int32_t)ch_11[1] * ch_11[1]);
  ch12p = ((int32_t)ch_12[0] * ch_12[0] + (int32_t)ch_12[1] * ch_12[1]);
  ch21p = ((int32_t)ch_21[0] * ch_21[0] + (int32_t)ch_21[1] * ch_21[1]);
  ch22p = ((int32_t)ch_22[0] * ch_22[0] + (int32_t)ch_22[1] * ch_22[1]);

  Qm = 2;
  symAmp = 10000;

  chPwr = ch11p + ch12p + ch21p + ch22p;

  a[0] = c[0] = 7071;
  b[0] = -2909; b[1] = 6444;
  d[0] = b[1]; d[1] = -b[0];

  MSE = (uint64_t **)calloc((1 << Qm), sizeof(uint64_t *));
  for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
      *(MSE + iter1) = (uint64_t *)calloc((1 << Qm), sizeof(uint64_t));
  }

  FILE * p = fopen("stbc.txt", "a");

  for (rb=0; rb<nb_rb; rb++) {
    for (re=0; re<((pilots==0)?12:8); re+=2) { 

      if (Qm == 2) {
        QAM_TABLE = QAM_2;
      } else if (Qm == 4) {
        QAM_TABLE = QAM_4;
      } else if (Qm == 6) {
        QAM_TABLE = QAM_6;
      }

      // Iterating over all possible value pairs of s3 and s4,
      // we calculate s1 and s2 using closed form equations and then
      // selec the pair (s1, s2, s3, s4) with minimum loss or squared error
      for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
        for (iter2 = 0; iter2 < (1 << Qm); iter2++) {
          //  Compute Squared Error

          // Assuming the s3 and s4 is known by iterating over every possible value
          s3[0] = QAM_TABLE[2 * iter1];
          s3[1] = QAM_TABLE[2 * iter1 + 1];
          s4[0] = QAM_TABLE[2 * iter2];
          s4[1] = QAM_TABLE[2 * iter2 + 1];

         // Calculating Equations (7a, 7b, 7c, 7d) from Paper
         
         // Calculating s3 * h_11
          z11[0] = (ch_11[0] * s3[0] - ch_11[1] * s3[1]) / symAmp;
          z11[1] = (ch_11[0] * s3[1] + ch_11[1] * s3[0]) / symAmp;

         // Calculating s4 * h_12
          z12[0] = (ch_12[0] * s4[0] - ch_12[1] * s4[1]) / symAmp;
          z12[1] = (ch_12[0] * s4[1] + ch_12[1] * s4[0]) / symAmp;

         // Calculating (s3 * h_11 + s4 * h_12)
          z1[0] = z11[0] + z12[0];
          z1[1] = z11[1] + z12[1];

         // Calculating b * (s3 * h_11 + s4 * h_12)
          z12[0] = ((z1[0] * b[0]) - (z1[1] * b[1])) / symAmp;
          z12[1] = ((z1[0] * b[1]) + (z1[1] * b[0])) / symAmp;

         // Calculating r1 - (b * (s3 * h_11 + s4 * h_12))
          z1[0] = rxF0[0] - z12[0];
          z1[1] = rxF0[1] - z12[1];

         // Calculating h_12 * conj(s3)
          z21[0] = (ch_12[0] * s3[0] - ch_12[1] * -s3[1]) / symAmp;
          z21[1] = (ch_12[0] * -s3[1] + ch_12[1] * s3[0]) / symAmp;

         // Calculating h_11 * conj(s4)
          z22[0] = (ch_11[0] * s4[0] - ch_11[1] * -s4[1]) / symAmp;
          z22[1] = (ch_11[0] * -s4[1] + ch_11[1] * s4[0]) / symAmp;

         // Calculating (h_12 * conj(s3) - h_11 * conj(s4))
          z21[0] = z21[0] - z22[0];
          z21[1] = z21[1] - z22[1];

         // Calculating d * (h_12 * conj(s3) - h_11 * conj(s4))
          z22[0] = ((z21[0] * d[0]) - (z21[1] * d[1])) / symAmp;
          z22[1] = ((z21[0] * d[1]) + (z21[1] * d[0])) / symAmp;

         // Calculating r2 - (d * (h_12 * conj(s3) - h_11 * conj(s4)))
          z2[0] = rxF0[2] - z22[0];
          z2[1] = rxF0[3] - z22[1];
          
         // Calculating h_21 * s3
          z31[0] = (ch_21[0] * s3[0] - ch_21[1] * s3[1]) / symAmp;
          z31[1] = (ch_21[0] * s3[1] + ch_21[1] * s3[0]) / symAmp;

         // Calculating h_22 * s4
          z32[0] = (ch_22[0] * s4[0] - ch_22[1] * s4[1]) / symAmp;
          z32[1] = (ch_22[0] * s4[1] + ch_22[1] * s4[0]) / symAmp;

         // Calculating (h_21 * s3) + (h_22 * s4)
          z31[0] = z31[0] + z32[0];
          z31[1] = z31[1] + z32[1];

         // Calculating b * ((h_21 * s3) + (h_22 * s4))
          z32[0] = ((z31[0] * b[0]) - (z31[1] * b[1])) / symAmp;
          z32[1] = ((z31[0] * b[1]) + (z31[1] * b[0])) / symAmp;      

         // Calculating r3 - (b * ((h_21 * s3) + (h_22 * s4)))
          z3[0] = rxF1[0] - z32[0];
          z3[1] = rxF1[1] - z32[1];

         // Calculating h_22 * conj(s3)
          z41[0] = (ch_22[0] * s3[0] - ch_22[1] * -s3[1]) / symAmp;
          z41[1] = (ch_22[0] * -s3[1] + ch_22[1] * s3[0]) / symAmp;
          
         // Calculating h_21 * conj(s4)
          z42[0] = (ch_21[0] * s4[0] - ch_21[1] * -s4[1]) / symAmp;
          z42[1] = (ch_21[0] * -s4[1] + ch_21[1] * s4[0]) / symAmp;
          
         // Calculating (h_22 * conj(s3) - h_21 * conj(s4))
          z41[0] = z41[0] - z42[0];
          z41[1] = z41[1] - z42[1];
          
         // Calculating d * (h_22 * conj(s3) - h_21 * conj(s4))
          z42[0] = ((z41[0] * d[0]) - (z41[1] * d[1])) / symAmp;
          z42[1] = ((z41[0] * d[1]) + (z41[1] * d[0])) / symAmp;    
          
         // Calculating r4 - d * (h_22 * conj(s3) - h_21 * conj(s4))
          z4[0] = rxF1[2] - z42[0];
          z4[1] = rxF1[3] - z42[1];
          
         // Calculating s1 based on equation (10)
          s1[0] = (ch_11[0] * z1[0] - (-ch_11[1]) * z1[1]);
          s1[1] = (ch_11[0] * z1[1] + (-ch_11[1]) * z1[0]);
          
          s1[0] += (ch_21[0] * z3[0] - (-ch_21[1]) * z3[1]);
          s1[1] += (ch_21[0] * z3[1] + (-ch_21[1]) * z3[0]);

          s1[0] += (ch_12[0] * z2[0] - ch_12[1] * -z2[1]);
          s1[1] += (ch_12[0] * -z2[1] + ch_12[1] * z2[0]);

          s1[0] += (ch_22[0] * z4[0] - ch_22[1] * -z4[1]);
          s1[1] += (ch_22[0] * -z4[1] + ch_22[1] * z4[0]);

          s1[0] = (int32_t)((int64_t)s1[0] * symAmp / a[0]);
          s1[1] = (int32_t)((int64_t)s1[1] * symAmp / a[0]);

          s1[0] = (int32_t)((int64_t)s1[0] * symAmp / chPwr);
          s1[1] = (int32_t)((int64_t)s1[1] * symAmp / chPwr);

         // Calculating s2 based on equation (11)
          s2[0] = (ch_12[0] * z1[0] - (-ch_12[1]) * z1[1]);
          s2[1] = (ch_12[0] * z1[1] + (-ch_12[1]) * z1[0]);

          s2[0] += (ch_22[0] * z3[0] - (-ch_22[1]) * z3[1]);
          s2[1] += (ch_22[0] * z3[1] + (-ch_22[1]) * z3[0]);

          s2[0] -= (ch_11[0] * z2[0] - ch_11[1] * -z2[1]);
          s2[1] -= (ch_11[0] * -z2[1] + ch_11[1] * z2[0]);

          s2[0] -= (ch_21[0] * z4[0] - ch_21[1] * -z4[1]);
          s2[1] -= (ch_21[0] * -z4[1] + ch_21[1] * z4[0]);

          s2[0] = (int32_t)((int64_t)s2[0] * symAmp / a[0]);
          s2[1] = (int32_t)((int64_t)s2[1] * symAmp / a[0]);

          s2[0] = (int32_t)((int64_t)s2[0] * symAmp / chPwr);
          s2[1] = (int32_t)((int64_t)s2[1] * symAmp / chPwr);

          // Now, for a certain pair of (s3, s4) - we calculated (s1, s2)
          // and will calculate the squared error with respect to the received
          // signal and channel conditions. The pair (s1, s2, s3, s4) which
          // results in least error is chosen as the received value given by
          // the definiton of "D(s1, s2, s3, s4)" in the paper on page 2.

          // Calculating s1 * a
          z11[0] = (a[0] * s1[0] - a[1] * s1[1]) / symAmp;
          z11[1] = (a[0] * s1[1] + a[1] * s1[0]) / symAmp;

          // Calculating b * s3
          z11_tmp[0] = (b[0] * s3[0] - b[1] * s3[1]) / symAmp;
          z11_tmp[1] = (b[0] * s3[1] + b[1] * s3[0]) / symAmp;

         // Calculating a * s1 + b * s3
          z11[0] = (z11[0] + z11_tmp[0]);
          z11[1] = (z11[1] + z11_tmp[1]);

          // Calculating h_11 * (a * s1 + b * s3)
          z11_tmp[0] = (ch_11[0] * z11[0] - ch_11[1] * z11[1]);
          z11_tmp[1] = (ch_11[0] * z11[1] + ch_11[1] * z11[0]);

          // Calculating a * s2
          z12[0] = (a[0] * s2[0] - a[1] * s2[1]) / symAmp;
          z12[1] = (a[0] * s2[1] + a[1] * s2[0]) / symAmp;

          // Calculating b * s4
          z12_tmp[0] = (b[0] * s4[0] - b[1] * s4[1]) / symAmp;
          z12_tmp[1] = (b[0] * s4[1] + b[1] * s4[0]) / symAmp;

         // Calculating (a * s2 + b * s4)
          z12[0] = (z12[0] + z12_tmp[0]);
          z12[1] = (z12[1] + z12_tmp[1]);

         // Calculating h_12 * (a * s2 + b * s4)
          z12_tmp[0] = (ch_12[0] * z12[0] - ch_12[1] * z12[1]);
          z12_tmp[1] = (ch_12[0] * z12[1] + ch_12[1] * z12[0]);

         // Calculating z1 = h_12 * (a * s2 + b * s4) + h_11 * (a * s1 + b * s3)
          z1[0] = z11_tmp[0] + z12_tmp[0];
          z1[1] = z11_tmp[1] + z12_tmp[1];
     
  // --------------------------------------------------------------------------------------------
         // Calculating c * conj(s1)
          z22[0] = (c[0] * s1[0] - c[1] * -s1[1]) / symAmp;
          z22[1] = (c[0] * -s1[1] + c[1] * s1[0]) / symAmp;

         // Calculating d * conj(s3)
          z22_tmp[0] = (d[0] * s3[0] - d[1] * -s3[1]) / symAmp;
          z22_tmp[1] = (d[0] * -s3[1] + d[1] * s3[0]) / symAmp;

         // Calculating (c * conj(s1) + d * conj(s3))
          z22[0] = (z22[0] + z22_tmp[0]);
          z22[1] = (z22[1] + z22_tmp[1]);

         // Calculating h_12 * (c * conj(s1) + d * conj(s3))
          z22_tmp[0] = (ch_12[0] * z22[0] - ch_12[1] * z22[1]);
          z22_tmp[1] = (ch_12[0] * z22[1] + ch_12[1] * z22[0]);

         // Calculating d * conj(s4)
          z21_tmp[0] = (d[0] * s4[0] - d[1] * -s4[1]) / symAmp;
          z21_tmp[1] = (d[0] * -s4[1] + d[1] * s4[0]) / symAmp;

         // Calculating c * conj(s2)
          z21[0] = (c[0] * s2[0] - c[1] * -s2[1]) / symAmp;
          z21[1] = (c[0] * -s2[1] + c[1] * s2[0]) / symAmp;

         // Calculating (c * conj(s2) + d * conj(s3))
          z21[0] = (z21[0] + z21_tmp[0]);
          z21[1] = (z21[1] + z21_tmp[1]);

         // Calculating h_11 * (c * conj(s2) + d * conj(s3))
          z21_tmp[0] = (ch_11[0] * z21[0] - ch_11[1] * z21[1]);
          z21_tmp[1] = (ch_11[0] * z21[1] + ch_11[1] * z21[0]);

         // Calculating z2 = h_12 * (c * conj(s1) + d * conj(s3)) - h_11 * (c * conj(s2) + d * conj(s3))
          z2[0] = z22_tmp[0] - z21_tmp[0];
          z2[1] = z22_tmp[1] - z21_tmp[1];

// -------------------------------------------  -------------------------
         // Calculating a * s1
          z31[0] = (a[0] * s1[0] - a[1] * s1[1]) / symAmp;
          z31[1] = (a[0] * s1[1] + a[1] * s1[0]) / symAmp;

         // Calculating b * s3
          z31_tmp[0] = (b[0] * s3[0] - b[1] * s3[1]) / symAmp;
          z31_tmp[1] = (b[0] * s3[1] + b[1] * s3[0]) / symAmp;

         // Calculating a * s1 + b * s3
          z31[0] = (z31[0] + z31_tmp[0]);
          z31[1] = (z31[1] + z31_tmp[1]);

         // Calculating h_21 * (a * s1 + b * s3)
          z31_tmp[0] = (ch_21[0] * z31[0] - ch_21[1] * z31[1]);
          z31_tmp[1] = (ch_21[0] * z31[1] + ch_21[1] * z31[0]);

         // Calculating a * s2
          z32[0] = (a[0] * s2[0] - a[1] * s2[1]) / symAmp;
          z32[1] = (a[0] * s2[1] + a[1] * s2[0]) / symAmp;

         // Calculating b * s4
          z32_tmp[0] = (b[0] * s4[0] - b[1] * s4[1]) / symAmp;
          z32_tmp[1] = (b[0] * s4[1] + b[1] * s4[0]) / symAmp;

         // Calculating a * s2 + b * s4
          z32[0] = (z32[0] + z32_tmp[0]);
          z32[1] = (z32[1] + z32_tmp[1]);

         // Calculating h_22 * (a * s2 + b * s4)
          z32_tmp[0] = (ch_22[0] * z32[0] - ch_22[1] * z32[1]);
          z32_tmp[1] = (ch_22[0] * z32[1] + ch_22[1] * z32[0]);

         // Calculating z3 = h_22 * (a * s2 + b * s4) + h_21 * (a * s1 + b * s3)
          z3[0] = z31_tmp[0] + z32_tmp[0];
          z3[1] = z31_tmp[1] + z32_tmp[1];

  // --------------------------------------------------------------------------------------------
         // Calculating c * conj(s1)
          z42[0] = (c[0] * s1[0] - c[1] * -s1[1]) / symAmp;
          z42[1] = (c[0] * -s1[1] + c[1] * s1[0]) / symAmp;

         // Calculating d * conj(s3)
          z42_tmp[0] = (d[0] * s3[0] - d[1] * -s3[1]) / symAmp;
          z42_tmp[1] = (d[0] * -s3[1] + d[1] * s3[0]) / symAmp;

         // Calculating (c * conj(s1) + d * conj(s3))
          z42[0] = (z42[0] + z42_tmp[0]);
          z42[1] = (z42[1] + z42_tmp[1]);

         // Calculating h_22 * (c * conj(s1) + d * conj(s3))
          z42_tmp[0] = (ch_22[0] * z42[0] - ch_22[1] * z42[1]);
          z42_tmp[1] = (ch_22[0] * z42[1] + ch_22[1] * z42[0]);

         // Calculating c * conj(s2)
          z41[0] = (c[0] * s2[0] - c[1] * -s2[1]) / symAmp;
          z41[1] = (c[0] * -s2[1] + c[1] * s2[0]) / symAmp;

         // Calculating d * conj(s4)
          z41_tmp[0] = (d[0] * s4[0] - d[1] * -s4[1]) / symAmp;
          z41_tmp[1] = (d[0] * -s4[1] + d[1] * s4[0]) / symAmp;

         // Calculating c * conj(s3) + d * conj(s4)
          z41[0] = (z41[0] + z41_tmp[0]);
          z41[1] = (z41[1] + z41_tmp[1]);

         // Calculating h_21 * (c * conj(s3) + d * conj(s4))
          z41_tmp[0] = (ch_21[0] * z41[0] - ch_21[1] * z41[1]);
          z41_tmp[1] = (ch_21[0] * z41[1] + ch_21[1] * z41[0]);

         // Calculating z4 = h_22 * (c * conj(s1) + d * conj(s3)) - h_21 * (c * conj(s3) + d * conj(s4))
          z4[0] = z42_tmp[0] - z41_tmp[0];
          z4[1] = z42_tmp[1] - z41_tmp[1];

          // Amplitude scaling for fixed point implementation
          z1[0] = z1[0] / symAmp;
          z1[1] = z1[1] / symAmp;
          z2[0] = z2[0] / symAmp;
          z2[1] = z2[1] / symAmp;
          z3[0] = z3[0] / symAmp;
          z3[1] = z3[1] / symAmp;
          z4[0] = z4[0] / symAmp;
          z4[1] = z4[1] / symAmp;
          

        // Calculating |Received signal - Channel * txSignal_hat| ^ 2
        // (I have reduced the amplitude of the error by 2^Qm to avoid overflow. This doesn't change the output)
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF0[0]) - z1[0]) >> (Qm)) * (uint64_t)(((int32_t)(rxF0[0]) - z1[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF0[1]) - z1[1]) >> (Qm)) * (uint64_t)(((int32_t)(rxF0[1]) - z1[1]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF0[2]) - z2[0]) >> (Qm)) * (uint64_t)(((int32_t)(rxF0[2]) - z2[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF0[3]) - z2[1]) >> (Qm)) * (uint64_t)(((int32_t)(rxF0[3]) - z2[1]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF1[0]) - z3[0]) >> (Qm)) * (uint64_t)(((int32_t)(rxF1[0]) - z3[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF1[1]) - z3[1]) >> (Qm)) * (uint64_t)(((int32_t)(rxF1[1]) - z3[1]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF1[2]) - z4[0]) >> (Qm)) * (uint64_t)(((int32_t)(rxF1[2]) - z4[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)(((int32_t)(rxF1[3]) - z4[1]) >> (Qm)) * (uint64_t)(((int32_t)(rxF1[3]) - z4[1]) >> (Qm));        
        }
      }

      // Find the (s3, s4) pair with least squared error
      min = MSE[0][0];

      for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
        for (iter2 = 0; iter2 < (1 << Qm); iter2++) {
          LOG_UI(PHY, "%d ", MSE[iter1][iter2]);
          if (MSE[iter1][iter2] < min) {
              min = MSE[iter1][iter2];
              x = iter1;
              y = iter2;
          }
          MSE[iter1][iter2] = 0;
        }
        LOG_UI(PHY, "\n");
      }

      qam_pt[0] = x;
      qam_pt[1] = y;

      // LOG_UI(PHY, "min QAM x: %d - y: %d\n", x, y);

      // (s3, s4) with least error
      s3[0] = QAM_TABLE[qam_pt[0] * 2];
      s3[1] = QAM_TABLE[qam_pt[0] * 2 + 1];
      s4[0] = QAM_TABLE[qam_pt[1] * 2];
      s4[1] = QAM_TABLE[qam_pt[1] * 2 + 1];

     // Calculating (s1, s2) as done previously but only for the (s3, s4) pair with least error
      // z11 = dlsch_stbc_mul(ch_11, s3, 0, 0);
      z11[0] = (ch_11[0] * s3[0] - ch_11[1] * s3[1]) / symAmp;
      z11[1] = (ch_11[0] * s3[1] + ch_11[1] * s3[0]) / symAmp;


      // z12 = dlsch_stbc_mul(ch_12, s4, 0, 0);
      z12[0] = (ch_12[0] * s4[0] - ch_12[1] * s4[1]) / symAmp;
      z12[1] = (ch_12[0] * s4[1] + ch_12[1] * s4[0]) / symAmp;

      // dlsch_stbc_add(z11, z12)
      z1[0] = z11[0] + z12[0];
      z1[1] = z11[1] + z12[1];

      //  dlsch_stbc_mul(b, z1, 0, 0)
      z12[0] = ((z1[0] * b[0]) - (z1[1] * b[1])) / symAmp;
      z12[1] = ((z1[0] * b[1]) + (z1[1] * b[0])) / symAmp;

      // z1 = dlsch_stbc_sub(rxF0, z12);
      z1[0] = rxF0[0] - z12[0];
      z1[1] = rxF0[1] - z12[1];

      // z21 = dlsch_stbc_mul(ch_12, s3, 0, 1);
      z21[0] = (ch_12[0] * s3[0] - ch_12[1] * -s3[1]) / symAmp;
      z21[1] = (ch_12[0] * -s3[1] + ch_12[1] * s3[0]) / symAmp;

      // z22 = dlsch_stbc_mul(ch_11, s4, 0, 1);
      z22[0] = (ch_11[0] * s4[0] - ch_11[1] * -s4[1]) / symAmp;
      z22[1] = (ch_11[0] * -s4[1] + ch_11[1] * s4[0]) / symAmp;
      z21[0] = z21[0] - z22[0];
      z21[1] = z21[1] - z22[1];

      // dlsch_stbc_mul(d, z21, 0, 0)
      z22[0] = ((z21[0] * d[0]) - (z21[1] * d[1])) / symAmp;
      z22[1] = ((z21[0] * d[1]) + (z21[1] * d[0])) / symAmp;
      // z2 = dlsch_stbc_sub(rxF0 + 2, z22);
      z2[0] = rxF0[2] - z22[0];
      z2[1] = rxF0[3] - z22[1];
      // z31 = dlsch_stbc_mul(ch_21, s3, 0, 0);
      z31[0] = (ch_21[0] * s3[0] - ch_21[1] * s3[1]) / symAmp;
      z31[1] = (ch_21[0] * s3[1] + ch_21[1] * s3[0]) / symAmp;
      // z32 = dlsch_stbc_mul(ch_22, s4, 0, 0);
      z32[0] = (ch_22[0] * s4[0] - ch_22[1] * s4[1]) / symAmp;
      z32[1] = (ch_22[0] * s4[1] + ch_22[1] * s4[0]) / symAmp;

      z31[0] = z31[0] + z32[0];
      z31[1] = z31[1] + z32[1];

      z32[0] = ((z31[0] * b[0]) - (z31[1] * b[1])) / symAmp;
      z32[1] = ((z31[0] * b[1]) + (z31[1] * b[0])) / symAmp;      

      z3[0] = rxF1[0] - z32[0];
      z3[1] = rxF1[1] - z32[1];


      z41[0] = (ch_22[0] * s3[0] - ch_22[1] * -s3[1]) / symAmp;
      z41[1] = (ch_22[0] * -s3[1] + ch_22[1] * s3[0]) / symAmp;


      // z42 = dlsch_stbc_mul(ch_21, s4, 0, 1);
      z42[0] = (ch_21[0] * s4[0] - ch_21[1] * -s4[1]) / symAmp;
      z42[1] = (ch_21[0] * -s4[1] + ch_21[1] * s4[0]) / symAmp;

      z41[0] = z41[0] - z42[0];
      z41[1] = z41[1] - z42[1];

      // dlsch_stbc_mul(d, z41, 0, 0)
      z42[0] = ((z41[0] * d[0]) - (z41[1] * d[1])) / symAmp;
      z42[1] = ((z41[0] * d[1]) + (z41[1] * d[0])) / symAmp;    

      // z4 = dlsch_stbc_sub(rxF1 + 2, z42);
      z4[0] = rxF1[2] - z42[0];
      z4[1] = rxF1[3] - z42[1];

      // s1 = dlsch_stbc_add(dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_11, z1, 1, 0), dlsch_stbc_mul(ch_21, z3, 1, 0)), a), dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_12, z2, 0, 1), dlsch_stbc_mul(ch_22, z4, 0, 1)), c));
      s1[0] = (ch_11[0] * z1[0] - (-ch_11[1]) * z1[1]);
      s1[1] = (ch_11[0] * z1[1] + (-ch_11[1]) * z1[0]);

      s1[0] += (ch_21[0] * z3[0] - (-ch_21[1]) * z3[1]);
      s1[1] += (ch_21[0] * z3[1] + (-ch_21[1]) * z3[0]);

      s1[0] += (ch_12[0] * z2[0] - ch_12[1] * -z2[1]);
      s1[1] += (ch_12[0] * -z2[1] + ch_12[1] * z2[0]);

      s1[0] += (ch_22[0] * z4[0] - ch_22[1] * -z4[1]);
      s1[1] += (ch_22[0] * -z4[1] + ch_22[1] * z4[0]);

      s1[0] = (int32_t)((int64_t)s1[0] * symAmp / a[0]);
      s1[1] = (int32_t)((int64_t)s1[1] * symAmp / a[0]);

      s1[0] = (int32_t)((int64_t)s1[0] * symAmp / chPwr);
      s1[1] = (int32_t)((int64_t)s1[1] * symAmp / chPwr);

      // s2 = dlsch_stbc_sub(dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_12, z1, 1, 0), dlsch_stbc_mul(ch_22, z3, 1, 0)), a), dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_11, z2, 0, 1), dlsch_stbc_mul(ch_21, z4, 0, 1)), c));
      s2[0] = (ch_12[0] * z1[0] - (-ch_12[1]) * z1[1]);
      s2[1] = (ch_12[0] * z1[1] + (-ch_12[1]) * z1[0]);

      s2[0] += (ch_22[0] * z3[0] - (-ch_22[1]) * z3[1]);
      s2[1] += (ch_22[0] * z3[1] + (-ch_22[1]) * z3[0]);

      s2[0] -= (ch_11[0] * z2[0] - ch_11[1] * -z2[1]);
      s2[1] -= (ch_11[0] * -z2[1] + ch_11[1] * z2[0]);

      s2[0] -= (ch_21[0] * z4[0] - ch_21[1] * -z4[1]);
      s2[1] -= (ch_21[0] * -z4[1] + ch_21[1] * z4[0]);

      s2[0] = (int32_t)((int64_t)s2[0] * symAmp / a[0]);
      s2[1] = (int32_t)((int64_t)s2[1] * symAmp / a[0]);

      s2[0] = (int32_t)((int64_t)s2[0] * symAmp / chPwr);
      s2[1] = (int32_t)((int64_t)s2[1] * symAmp / chPwr);

      s1[0] = s1[0] * 180 / 7071;
      s1[1] = s1[1] * 180 / 7071;
      s2[0] = s2[0] * 180 / 7071;
      s2[1] = s2[1] * 180 / 7071;
      s3[0] = s3[0] * 180 / 7071;
      s3[1] = s3[1] * 180 / 7071;
      s4[0] = s4[0] * 180 / 7071;
      s4[1] = s4[1] * 180 / 7071;


      rxF0_comp[0] = (int16_t)s1[0];
      rxF0_comp[1] = (int16_t)s1[1];
      rxF0_comp[2] = (int16_t)s2[0];
      rxF0_comp[3] = (int16_t)s2[1];
      rxF0_comp[4] = (int16_t)s3[0];
      rxF0_comp[5] = (int16_t)s3[1];
      rxF0_comp[6] = (int16_t)s4[0];
      rxF0_comp[7] = (int16_t)s4[1];

      fprintf(p, "symbol: %d | offset: %d | NRB: %d | re: %d | %d %d\n", symbol, (symbol * 12 * 25), rb, re, rxF0_comp[0], rxF0_comp[1]);
      fprintf(p, "symbol: %d | offset: %d | NRB: %d | re: %d | %d %d\n", symbol, (symbol * 12 * 25), rb, re, rxF0_comp[2], rxF0_comp[3]);
      fprintf(p, "symbol: %d | offset: %d | NRB: %d | re: %d | %d %d\n", symbol, (symbol * 12 * 25), rb, re+1, rxF0_comp[4], rxF0_comp[5]);
      fprintf(p, "symbol: %d | offset: %d | NRB: %d | re: %d | %d %d\n", symbol, (symbol * 12 * 25), rb, re+1, rxF0_comp[6], rxF0_comp[7]);

      rxF0_comp += 8;
      rxF0 += 4;
      rxF1 += 4;
      ch_11 += 2;
      ch_12 += 2;
      ch_21 += 2;
      ch_22 += 2;
      
    }
  }
  fclose(p);


  free(s1);
  free(s2);
  free(s3);
  free(s4);
  free(z1);
  free(z11);
  free(z11_tmp);
  free(z12);
  free(z12_tmp);
  free(z2);
  free(z21);
  free(z21_tmp);
  free(z22);
  free(z22_tmp);
  free(z3);
  free(z31);
  free(z31_tmp);
  free(z32);
  free(z32_tmp);
  free(z4);
  free(z41);
  free(z41_tmp);
  free(z42);
  free(z42_tmp);

  for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
    free(MSE[iter1]);
  }

  free(MSE);

  free(a);
  free(b);
  free(c);
  free(d);

  _mm_empty();
  _m_empty();
}
