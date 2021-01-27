void dlsch_rx_stbc(LTE_DL_FRAME_PARMS *frame_parms,
                          int ** rxdataF_ext,
                          int ** rxdataF_comp,
                          int **dl_ch_ext,
                          unsigned char symbol,
                          unsigned short nb_rb,
                          uint8_t Qm) {

  // NEED HARDCODED QAM TABLE FOR 4-QAM, 16-QAM, and 64-QAM.
  
  int32_t QAM_2[8] = {-23170 ,23170 ,-23170 ,-23170 ,23170 ,23170 ,23170 ,-23170};
  int32_t QAM_4[32] = {-31086,31086,-31086,10362,-31086,-31086,-31086,-10362,-10362,31086,-10362,10362,-10362,-31086,-10362,-10362,31086,31086,31086,10362,31086,-31086,31086,-10362,10362,31086,10362,10362,10362,-31086,10362,-10362};
  int32_t QAM_6[128] = {-35393,35393,-35393,25281,-35393,5056,-35393,15168,-35393,-35393,-35393,-25281,-35393,-5056,-35393,-15168,-25281,35393,-25281,25281,-25281,5056,-25281,15168,-25281,-35393,-25281,-25281,-25281,-5056,-25281,-15168,-5056,35393,-5056,25281,-5056,5056,-5056,15168,-5056,-35393,-5056,-25281,-5056,-5056,-5056,-15168,-15168,35393,-15168,25281,-15168,5056,-15168,15168,-15168,-35393,-15168,-25281,-15168,-5056,-15168,-15168,35393,35393,35393,25281,35393,5056,35393,15168,35393,-35393,35393,-25281,35393,-5056,35393,-15168,25281,35393,25281,25281,25281,5056,25281,15168,25281,-35393,25281,-25281,25281,-5056,25281,-15168,5056,35393,5056,25281,5056,5056,5056,15168,5056,-35393,5056,-25281,5056,-5056,5056,-15168,15168,35393,15168,25281,15168,5056,15168,15168,15168,-35393,15168,-25281,15168,-5056,15168,-15168};

  int32_t *rxF0,*rxF1;
  int32_t * QAM_TABLE;
  int qam_pt[2] = {0, 0};
  // int32_t *ch_11,*ch_12,*ch_21,*ch_22;
  // int32_t *z1, *z11, *z11_tmp, *z12, *z12_tmp, *z2, *z21, *z21_tmp, *z22, *z22_tmp, *z3, *z31, *z31_tmp, *z32, *z32_tmp, *z4, *z41, *z41_tmp, *z42, *z42_tmp;
  uint64_t ** MSE;
  int32_t chPwr = 0;
  int iter1, iter2,x=0,y=0;
  uint64_t min;
  unsigned char rb,re;
  int jj = (symbol*frame_parms->N_RB_DL*12);
  uint8_t symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;
  uint8_t pilots = ((symbol_mod==0)||(symbol_mod==(4-frame_parms->Ncp))) ? 1 : 0;
  int32_t * rxF0_comp = (int32_t *) &rxdataF_comp[0][jj];

  //amp = _mm_set1_epi16(ONE_OVER_2_Q15);

  rxF0     = (int32_t *)&rxdataF_ext[0][jj];
  rxF1     = (int32_t *)&rxdataF_ext[1][jj];

  // For 2-bit truth Table  
  // rxF0[0] = -54440;
  // rxF0[1] = 35160;
  // rxF0[2] = 18770;
  // rxF0[3] = -87210;
  // rxF1[0] = -84310;
  // rxF1[1] = 54440;
  // rxF1[2] = 21670;
  // rxF1[3] = -100690;

  rxF0[0] = 3671000;
  rxF0[1] = 4155300;
  rxF0[2] = 9400;
  rxF0[3] = -1324200;
  rxF1[0] = 4272300;
  rxF1[1] = 2555900;
  rxF1[2] = -289300;
  rxF1[3] = -956700;

  Qm = 6;
  
  // rxF0[0] = 
  int32_t ch_11[2] = {21, 97}; // (int32_t *)&dl_ch_ext[0][jj];
  int32_t ch_12[2] = {78, 66}; // (int32_t *)&dl_ch_ext[1][jj];
  int32_t ch_21[2] = {45, 74}; // (int32_t *)&dl_ch_ext[2][jj];
  int32_t ch_22[2] = {91, 28}; // (int32_t *)&dl_ch_ext[3][jj];

  int32_t chAvg = 100; // abs(max(ch11, ch12, ch21, ch22))

  rxF0[0] = rxF0[0] / chAvg; 
  rxF0[1] = rxF0[1] / chAvg; 
  rxF0[2] = rxF0[2] / chAvg; 
  rxF0[3] = rxF0[3] / chAvg; 
  rxF1[0] = rxF1[0] / chAvg; 
  rxF1[1] = rxF1[1] / chAvg; 
  rxF1[2] = rxF1[2] / chAvg; 
  rxF1[3] = rxF1[3] / chAvg; 

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

  // HOW TO IMPLEMENT THIS USING INTEGERS
  a[0] = c[0] = 7071;
  b[0] = -2909; b[1] = 6444;
  d[0] = b[1]; d[1] = -b[0];

  chPwr = ch_11[0] * ch_11[0] + ch_11[1] * ch_11[1];
  chPwr += ch_12[0] * ch_12[0] + ch_12[1] * ch_12[1];
  chPwr += ch_21[0] * ch_21[0] + ch_21[1] * ch_21[1];
  chPwr += ch_22[0] * ch_22[0] + ch_22[1] * ch_22[1];

  chPwr = chPwr * 1000;
  chPwr = chPwr / (chAvg * chAvg);

  for (rb=0; rb<nb_rb; rb++) {
    for (re=0; re<((pilots==0)?12:8); re+=2) { 

      MSE = (uint64_t **)calloc((1 << Qm), sizeof(uint64_t *));
      for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
          *(MSE + iter1) = (uint64_t *)calloc((1 << Qm), sizeof(uint64_t));
      }

      if (Qm == 2) {
        QAM_TABLE = QAM_2;
      } else if (Qm == 4) {
        QAM_TABLE = QAM_4;
      } else if (Qm == 6) {
        QAM_TABLE = QAM_6;
      }

      // Bruteforce Search over QAM constellations for two symbols
      for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
        for (iter2 = 0; iter2 < (1 << Qm); iter2++) {
          //  Compute Squared Error

          s3[0] = QAM_TABLE[2 * iter1];
          s3[1] = QAM_TABLE[2 * iter1 + 1];
          s4[0] = QAM_TABLE[2 * iter2];
          s4[1] = QAM_TABLE[2 * iter2 + 1];

          // LOG_UI(PHY, "s3: %d %d\n", s3[0], s3[1]);
          
          // LOG_UI(PHY, "s4: %d %d\n", s4[0], s4[1]);

          // LOG_UI(PHY, "ch_11: %d %d\n", ch_11[0], ch_11[1]);
          
          // LOG_UI(PHY, "ch_12: %d %d\n", ch_12[0], ch_12[1]);

          // z11 = dlsch_stbc_mul(ch_11, s3, 0, 0);
          z11[0] = (ch_11[0] * s3[0] - ch_11[1] * s3[1]) / chAvg;
          z11[1] = (ch_11[0] * s3[1] + ch_11[1] * s3[0]) / chAvg;

          // LOG_UI(PHY, "z11: %d %d\n", z11[0], z11[1]);
          
          // z12 = dlsch_stbc_mul(ch_12, s4, 0, 0);
          z12[0] = (ch_12[0] * s4[0] - ch_12[1] * s4[1]) / chAvg;
          z12[1] = (ch_12[0] * s4[1] + ch_12[1] * s4[0]) / chAvg;
          // LOG_UI(PHY, "z12: %d %d\n", z12[0], z12[1]);

          // dlsch_stbc_add(z11, z12)
          z1[0] = z11[0] + z12[0];
          z1[1] = z11[1] + z12[1];
          // LOG_UI(PHY, "z1: %d %d\n", z1[0], z1[1]);

          //  dlsch_stbc_mul(b, z1, 0, 0)
          z12[0] = ((z1[0] * b[0]) - (z1[1] * b[1])) / 10000;
          z12[1] = ((z1[0] * b[1]) + (z1[1] * b[0])) / 10000;
          // LOG_UI(PHY, "z12: %d %d\n", z12[0], z12[1]);

          // z1 = dlsch_stbc_sub(rxF0, z12);
          z1[0] = rxF0[0] - z12[0];
          z1[1] = rxF0[1] - z12[1];
          // LOG_UI(PHY, "rxF0: %d %d\n", rxF0[0], rxF0[1]);
          // LOG_UI(PHY, "z1: %d %d\n", z1[0], z1[1]);
          // exit(0);
          // z21 = dlsch_stbc_mul(ch_12, s3, 0, 1);
          z21[0] = (ch_12[0] * s3[0] - ch_12[1] * -s3[1]) / chAvg;
          z21[1] = (ch_12[0] * -s3[1] + ch_12[1] * s3[0]) / chAvg;

          // z22 = dlsch_stbc_mul(ch_11, s4, 0, 1);
          z22[0] = (ch_11[0] * s4[0] - ch_11[1] * -s4[1]) / chAvg;
          z22[1] = (ch_11[0] * -s4[1] + ch_11[1] * s4[0]) / chAvg;
          z21[0] = z21[0] - z22[0];
          z21[1] = z21[1] - z22[1];

          // dlsch_stbc_mul(d, z21, 0, 0)
          z22[0] = ((z21[0] * d[0]) - (z21[1] * d[1])) / 10000;
          z22[1] = ((z21[0] * d[1]) + (z21[1] * d[0])) / 10000;
          // z2 = dlsch_stbc_sub(rxF0 + 2, z22);
          z2[0] = rxF0[2] - z22[0];
          z2[1] = rxF0[3] - z22[1];
          // z31 = dlsch_stbc_mul(ch_21, s3, 0, 0);
          z31[0] = (ch_21[0] * s3[0] - ch_21[1] * s3[1]) / chAvg;
          z31[1] = (ch_21[0] * s3[1] + ch_21[1] * s3[0]) / chAvg;
          // z32 = dlsch_stbc_mul(ch_22, s4, 0, 0);
          z32[0] = (ch_22[0] * s4[0] - ch_22[1] * s4[1]) / chAvg;
          z32[1] = (ch_22[0] * s4[1] + ch_22[1] * s4[0]) / chAvg;

          z31[0] = z31[0] + z32[0];
          z31[1] = z31[1] + z32[1];

          z32[0] = ((z31[0] * b[0]) - (z31[1] * b[1])) / 10000;
          z32[1] = ((z31[0] * b[1]) + (z31[1] * b[0])) / 10000;      

          z3[0] = rxF1[0] - z32[0];
          z3[1] = rxF1[1] - z32[1];


          z41[0] = (ch_22[0] * s3[0] - ch_22[1] * -s3[1]) / chAvg;
          z41[1] = (ch_22[0] * -s3[1] + ch_22[1] * s3[0]) / chAvg;
          
          
          // z42 = dlsch_stbc_mul(ch_21, s4, 0, 1);
          z42[0] = (ch_21[0] * s4[0] - ch_21[1] * -s4[1]) / chAvg;
          z42[1] = (ch_21[0] * -s4[1] + ch_21[1] * s4[0]) / chAvg;
          
          z41[0] = z41[0] - z42[0];
          z41[1] = z41[1] - z42[1];
          
          // dlsch_stbc_mul(d, z41, 0, 0)
          z42[0] = ((z41[0] * d[0]) - (z41[1] * d[1])) / 10000;
          z42[1] = ((z41[0] * d[1]) + (z41[1] * d[0])) / 10000;    
          
          // z4 = dlsch_stbc_sub(rxF1 + 2, z42);
          z4[0] = rxF1[2] - z42[0];
          z4[1] = rxF1[3] - z42[1];
          
          // s1 = dlsch_stbc_add(dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_11, z1, 1, 0), dlsch_stbc_mul(ch_21, z3, 1, 0)), a), dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_12, z2, 0, 1), dlsch_stbc_mul(ch_22, z4, 0, 1)), c));
          s1[0] = (ch_11[0] * z1[0] - (-ch_11[1]) * z1[1]) / chAvg;
          s1[1] = (ch_11[0] * z1[1] + (-ch_11[1]) * z1[0]) / chAvg;

          s1[0] += (ch_21[0] * z3[0] - (-ch_21[1]) * z3[1]) / chAvg;
          s1[1] += (ch_21[0] * z3[1] + (-ch_21[1]) * z3[0]) / chAvg;

          s1[0] += (ch_12[0] * z2[0] - ch_12[1] * -z2[1]) / chAvg;
          s1[1] += (ch_12[0] * -z2[1] + ch_12[1] * z2[0]) / chAvg;

          s1[0] += (ch_22[0] * z4[0] - ch_22[1] * -z4[1]) / chAvg;
          s1[1] += (ch_22[0] * -z4[1] + ch_22[1] * z4[0]) / chAvg;

          s1[0] = (s1[0]) * 10000;
          s1[1] = (s1[1]) * 10000;

          s1[0] = s1[0] / a[0];
          s1[1] = s1[1] / a[0];

          s1[0] = (s1[0]) * 1000;
          s1[1] = (s1[1]) * 1000;

          s1[0] = s1[0] / chPwr;
          s1[1] = s1[1] / chPwr;

          // s1[0] = (s1[0]) * chAvg;
          // s1[1] = (s1[1]) * chAvg;

          // s2 = dlsch_stbc_sub(dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_12, z1, 1, 0), dlsch_stbc_mul(ch_22, z3, 1, 0)), a), dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_11, z2, 0, 1), dlsch_stbc_mul(ch_21, z4, 0, 1)), c));
          s2[0] = (ch_12[0] * z1[0] - (-ch_12[1]) * z1[1]) / chAvg;
          s2[1] = (ch_12[0] * z1[1] + (-ch_12[1]) * z1[0]) / chAvg;

          s2[0] += (ch_22[0] * z3[0] - (-ch_22[1]) * z3[1]) / chAvg;
          s2[1] += (ch_22[0] * z3[1] + (-ch_22[1]) * z3[0]) / chAvg;

          s2[0] -= (ch_11[0] * z2[0] - ch_11[1] * -z2[1]) / chAvg;
          s2[1] -= (ch_11[0] * -z2[1] + ch_11[1] * z2[0]) / chAvg;

          s2[0] -= (ch_21[0] * z4[0] - ch_21[1] * -z4[1]) / chAvg;
          s2[1] -= (ch_21[0] * -z4[1] + ch_21[1] * z4[0]) / chAvg;

          s2[0] = (s2[0]) * 10000;
          s2[1] = (s2[1]) * 10000;

          s2[0] = s2[0] / a[0];
          s2[1] = s2[1] / a[0];


          s2[0] = (s2[0]) * 1000;
          s2[1] = (s2[1]) * 1000;

          s2[0] = s2[0] / chPwr;
          s2[1] = s2[1] / chPwr;
          // LOG_UI(PHY, "s1: %d %d\n", s1[0], s1[1]);
          // LOG_UI(PHY, "s2: %d %d\n", s2[0], s2[1]);
          // LOG_UI(PHY, "---------------------------\n");


          // z11 = dlsch_stbc_mul(a, s1, 0, 0);
          z11[0] = (a[0] * s1[0] - a[1] * s1[1]) / 10000;
          z11[1] = (a[0] * s1[1] + a[1] * s1[0]) / 10000;

          // z11_tmp = b * s3
          z11_tmp[0] = (b[0] * s3[0] - b[1] * s3[1]) / 10000;
          z11_tmp[1] = (b[0] * s3[1] + b[1] * s3[0]) / 10000;

          z11[0] = (z11[0] + z11_tmp[0]);
          z11[1] = (z11[1] + z11_tmp[1]);

          //  z11 = z11 * ch_11
          z11_tmp[0] = (ch_11[0] * z11[0] - ch_11[1] * z11[1]) / chAvg;
          z11_tmp[1] = (ch_11[0] * z11[1] + ch_11[1] * z11[0]) / chAvg;

          // z11 = dlsch_stbc_mul(a, s1, 0, 0);
          z12[0] = (a[0] * s2[0] - a[1] * s2[1]) / 10000;
          z12[1] = (a[0] * s2[1] + a[1] * s2[0]) / 10000;

          // z12_tmp = b * s3
          z12_tmp[0] = (b[0] * s4[0] - b[1] * s4[1]) / 10000;
          z12_tmp[1] = (b[0] * s4[1] + b[1] * s4[0]) / 10000;

          z12[0] = (z12[0] + z12_tmp[0]);
          z12[1] = (z12[1] + z12_tmp[1]);

          //  z12 = z12 * ch_12
          z12_tmp[0] = (ch_12[0] * z12[0] - ch_12[1] * z12[1]) / chAvg;
          z12_tmp[1] = (ch_12[0] * z12[1] + ch_12[1] * z12[0]) / chAvg;

          z1[0] = z11_tmp[0] + z12_tmp[0];
          z1[1] = z11_tmp[1] + z12_tmp[1];
     
  // --------------------------------------------------------------------------------------------
          z22[0] = (c[0] * s1[0] - c[1] * -s1[1]) / 10000;
          z22[1] = (c[0] * -s1[1] + c[1] * s1[0]) / 10000;
          // LOG_UI(PHY, "z22: %d %d\n", z22[0], z22[1]);

          // z22_tmp = b * s3
          z22_tmp[0] = (d[0] * s3[0] - d[1] * -s3[1]) / 10000;
          z22_tmp[1] = (d[0] * -s3[1] + d[1] * s3[0]) / 10000;
          // LOG_UI(PHY, "z22_tmp: %d %d\n", z22_tmp[0], z22_tmp[1]);

          z22[0] = (z22[0] + z22_tmp[0]);
          z22[1] = (z22[1] + z22_tmp[1]);
          // LOG_UI(PHY, "z22: %d %d\n", z22[0], z22[1]);

          //  z11 = z11 * ch_11
          z22_tmp[0] = (ch_12[0] * z22[0] - ch_12[1] * z22[1]) / chAvg;
          z22_tmp[1] = (ch_12[0] * z22[1] + ch_12[1] * z22[0]) / chAvg;
          // LOG_UI(PHY, "z22_tmp: %d %d\n", z22_tmp[0], z22_tmp[1]);
  

          z21_tmp[0] = (d[0] * s4[0] - d[1] * -s4[1]) / 10000;
          z21_tmp[1] = (d[0] * -s4[1] + d[1] * s4[0]) / 10000;
          // LOG_UI(PHY, "z21_tmp: %d %d\n", z21_tmp[0], z21_tmp[1]);

          z21[0] = (c[0] * s2[0] - c[1] * -s2[1]) / 10000;
          z21[1] = (c[0] * -s2[1] + c[1] * s2[0]) / 10000;
          // LOG_UI(PHY, "z21: %d %d\n", z21[0], z21[1]);

          z21[0] = (z21[0] + z21_tmp[0]);
          z21[1] = (z21[1] + z21_tmp[1]);
          // LOG_UI(PHY, "z21: %d %d\n", z21[0], z21[1]);

          //  z11 = z11 * ch_11
          z21_tmp[0] = (ch_11[0] * z21[0] - ch_11[1] * z21[1]) / chAvg;
          z21_tmp[1] = (ch_11[0] * z21[1] + ch_11[1] * z21[0]) / chAvg;
          // LOG_UI(PHY, "z21_tmp: %d %d\n", z21_tmp[0], z21_tmp[1]);

          // LOG_UI(PHY, "z21_tmp: %d %d\n", z21_tmp[0], z21_tmp[1]);
          // LOG_UI(PHY, "z22_tmp: %d %d\n", z22_tmp[0], z22_tmp[1]);
          z2[0] = z22_tmp[0] - z21_tmp[0];
          z2[1] = z22_tmp[1] - z21_tmp[1];
          // LOG_UI(PHY, "z2: %d %d\n", z2[0], z2[1]);
          // exit(0);
// -------------------------------------------  -------------------------
          z31[0] = (a[0] * s1[0] - a[1] * s1[1]) / 10000;
          z31[1] = (a[0] * s1[1] + a[1] * s1[0]) / 10000;

          // z31_tmp = b * s3
          z31_tmp[0] = (b[0] * s3[0] - b[1] * s3[1]) / 10000;
          z31_tmp[1] = (b[0] * s3[1] + b[1] * s3[0]) / 10000;

          z31[0] = (z31[0] + z31_tmp[0]);
          z31[1] = (z31[1] + z31_tmp[1]);

          //  z31 = z31 * ch_11
          z31_tmp[0] = (ch_21[0] * z31[0] - ch_21[1] * z31[1]) / chAvg;
          z31_tmp[1] = (ch_21[0] * z31[1] + ch_21[1] * z31[0]) / chAvg;

          // z31 = dlsch_stbc_mul(a, s1, 0, 0);
          z32[0] = (a[0] * s2[0] - a[1] * s2[1]) / 10000;
          z32[1] = (a[0] * s2[1] + a[1] * s2[0]) / 10000;

          // z32_tmp = b * s3
          z32_tmp[0] = (b[0] * s4[0] - b[1] * s4[1]) / 10000;
          z32_tmp[1] = (b[0] * s4[1] + b[1] * s4[0]) / 10000;

          z32[0] = (z32[0] + z32_tmp[0]);
          z32[1] = (z32[1] + z32_tmp[1]);

          //  z32 = z32 * ch_12
          z32_tmp[0] = (ch_22[0] * z32[0] - ch_22[1] * z32[1]) / chAvg;
          z32_tmp[1] = (ch_22[0] * z32[1] + ch_22[1] * z32[0]) / chAvg;

          z3[0] = z31_tmp[0] + z32_tmp[0];
          z3[1] = z31_tmp[1] + z32_tmp[1];

  // --------------------------------------------------------------------------------------------
          z42[0] = (c[0] * s1[0] - c[1] * -s1[1]) / 10000;
          z42[1] = (c[0] * -s1[1] + c[1] * s1[0]) / 10000;

          // z42_tmp = b * s3
          z42_tmp[0] = (d[0] * s3[0] - d[1] * -s3[1]) / 10000;
          z42_tmp[1] = (d[0] * -s3[1] + d[1] * s3[0]) / 10000;

          z42[0] = (z42[0] + z42_tmp[0]);
          z42[1] = (z42[1] + z42_tmp[1]);

          //  z11 = z11 * ch_11
          z42_tmp[0] = (ch_22[0] * z42[0] - ch_22[1] * z42[1]) / chAvg;
          z42_tmp[1] = (ch_22[0] * z42[1] + ch_22[1] * z42[0]) / chAvg;

          z41[0] = (c[0] * s2[0] - c[1] * -s2[1]) / 10000;
          z41[1] = (c[0] * -s2[1] + c[1] * s2[0]) / 10000;

          // z41_tmp = b * s3
          z41_tmp[0] = (d[0] * s4[0] - d[1] * -s4[1]) / 10000;
          z41_tmp[1] = (d[0] * -s4[1] + d[1] * s4[0]) / 10000;

          z41[0] = (z41[0] + z41_tmp[0]);
          z41[1] = (z41[1] + z41_tmp[1]);

          //  z11 = z11 * ch_11
          z41_tmp[0] = (ch_21[0] * z41[0] - ch_21[1] * z41[1]) / chAvg;
          z41_tmp[1] = (ch_21[0] * z41[1] + ch_21[1] * z41[0]) / chAvg;

          z4[0] = z42_tmp[0] - z41_tmp[0];
          z4[1] = z42_tmp[1] - z41_tmp[1];

          // LOG_UI(PHY, "z1: %d %d\n", z1[0], z1[1]);
          // LOG_UI(PHY, "z2: %d %d\n", z2[0], z2[1]);
          // LOG_UI(PHY, "z3: %d %d\n", z3[0], z3[1]);
          // LOG_UI(PHY, "z4: %d %d\n", z4[0], z4[1]);
          // LOG_UI(PHY, "---------------------------\n");

          // exit(0);

          MSE[iter1][iter2] += (uint64_t)((rxF0[0] - z1[0]) >> (Qm)) * (uint64_t)((rxF0[0] - z1[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF0[1] - z1[1]) >> (Qm)) * (uint64_t)((rxF0[1] - z1[1]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF0[2] - z2[0]) >> (Qm)) * (uint64_t)((rxF0[2] - z2[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF0[3] - z2[1]) >> (Qm)) * (uint64_t)((rxF0[3] - z2[1]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF1[0] - z3[0]) >> (Qm)) * (uint64_t)((rxF1[0] - z3[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF1[1] - z3[1]) >> (Qm)) * (uint64_t)((rxF1[1] - z3[1]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF1[2] - z4[0]) >> (Qm)) * (uint64_t)((rxF1[2] - z4[0]) >> (Qm));
          MSE[iter1][iter2] += (uint64_t)((rxF1[3] - z4[1]) >> (Qm)) * (uint64_t)((rxF1[3] - z4[1]) >> (Qm));        
        }
      }

      // Find the(iter_qam1, iter_qam2) combination with least squared error
      min = MSE[0][0];

      for (iter1 = 0; iter1 < (1 << Qm); iter1++) {
        for (iter2 = 0; iter2 < (1 << Qm); iter2++) {
          LOG_UI(PHY, "%d ", MSE[iter1][iter2]);
          if (MSE[iter1][iter2] < min) {
              min = MSE[iter1][iter2];
              x = iter1;
              y = iter2;
          }
        }
        LOG_UI(PHY, "\n");
      }

      qam_pt[0] = x;
      qam_pt[1] = y;

      LOG_UI(PHY, "min QAM x: %d - y: %d\n", x, y);
      // Rest two symbols have a closed form solutions given that first two symbols are known

      s3[0] = QAM_TABLE[qam_pt[0] * 2];
      s3[1] = QAM_TABLE[qam_pt[0] * 2 + 1];
      s4[0] = QAM_TABLE[qam_pt[1] * 2];
      s4[1] = QAM_TABLE[qam_pt[1] * 2 + 1];

      // z11 = dlsch_stbc_mul(ch_11, s3, 0, 0);
      z11[0] = (ch_11[0] * s3[0] - ch_11[1] * s3[1]) / chAvg;
      z11[1] = (ch_11[0] * s3[1] + ch_11[1] * s3[0]) / chAvg;


      // z12 = dlsch_stbc_mul(ch_12, s4, 0, 0);
      z12[0] = (ch_12[0] * s4[0] - ch_12[1] * s4[1]) / chAvg;
      z12[1] = (ch_12[0] * s4[1] + ch_12[1] * s4[0]) / chAvg;

      // dlsch_stbc_add(z11, z12)
      z1[0] = z11[0] + z12[0];
      z1[1] = z11[1] + z12[1];

      //  dlsch_stbc_mul(b, z1, 0, 0)
      z12[0] = ((z1[0] * b[0]) - (z1[1] * b[1])) / 10000;
      z12[1] = ((z1[0] * b[1]) + (z1[1] * b[0])) / 10000;

      // z1 = dlsch_stbc_sub(rxF0, z12);
      z1[0] = rxF0[0] - z12[0];
      z1[1] = rxF0[1] - z12[1];
      // exit(0);
      // z21 = dlsch_stbc_mul(ch_12, s3, 0, 1);
      z21[0] = (ch_12[0] * s3[0] - ch_12[1] * -s3[1]) / chAvg;
      z21[1] = (ch_12[0] * -s3[1] + ch_12[1] * s3[0]) / chAvg;

      // z22 = dlsch_stbc_mul(ch_11, s4, 0, 1);
      z22[0] = (ch_11[0] * s4[0] - ch_11[1] * -s4[1]) / chAvg;
      z22[1] = (ch_11[0] * -s4[1] + ch_11[1] * s4[0]) / chAvg;
      z21[0] = z21[0] - z22[0];
      z21[1] = z21[1] - z22[1];

      // dlsch_stbc_mul(d, z21, 0, 0)
      z22[0] = ((z21[0] * d[0]) - (z21[1] * d[1])) / 10000;
      z22[1] = ((z21[0] * d[1]) + (z21[1] * d[0])) / 10000;
      // z2 = dlsch_stbc_sub(rxF0 + 2, z22);
      z2[0] = rxF0[2] - z22[0];
      z2[1] = rxF0[3] - z22[1];
      // z31 = dlsch_stbc_mul(ch_21, s3, 0, 0);
      z31[0] = (ch_21[0] * s3[0] - ch_21[1] * s3[1]) / chAvg;
      z31[1] = (ch_21[0] * s3[1] + ch_21[1] * s3[0]) / chAvg;
      // z32 = dlsch_stbc_mul(ch_22, s4, 0, 0);
      z32[0] = (ch_22[0] * s4[0] - ch_22[1] * s4[1]) / chAvg;
      z32[1] = (ch_22[0] * s4[1] + ch_22[1] * s4[0]) / chAvg;

      z31[0] = z31[0] + z32[0];
      z31[1] = z31[1] + z32[1];

      z32[0] = ((z31[0] * b[0]) - (z31[1] * b[1])) / 10000;
      z32[1] = ((z31[0] * b[1]) + (z31[1] * b[0])) / 10000;      

      z3[0] = rxF1[0] - z32[0];
      z3[1] = rxF1[1] - z32[1];


      z41[0] = (ch_22[0] * s3[0] - ch_22[1] * -s3[1]) / chAvg;
      z41[1] = (ch_22[0] * -s3[1] + ch_22[1] * s3[0]) / chAvg;


      // z42 = dlsch_stbc_mul(ch_21, s4, 0, 1);
      z42[0] = (ch_21[0] * s4[0] - ch_21[1] * -s4[1]) / chAvg;
      z42[1] = (ch_21[0] * -s4[1] + ch_21[1] * s4[0]) / chAvg;

      z41[0] = z41[0] - z42[0];
      z41[1] = z41[1] - z42[1];

      // dlsch_stbc_mul(d, z41, 0, 0)
      z42[0] = ((z41[0] * d[0]) - (z41[1] * d[1])) / 10000;
      z42[1] = ((z41[0] * d[1]) + (z41[1] * d[0])) / 10000;    

      // z4 = dlsch_stbc_sub(rxF1 + 2, z42);
      z4[0] = rxF1[2] - z42[0];
      z4[1] = rxF1[3] - z42[1];


      // s1 = dlsch_stbc_add(dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_11, z1, 1, 0), dlsch_stbc_mul(ch_21, z3, 1, 0)), a), dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_12, z2, 0, 1), dlsch_stbc_mul(ch_22, z4, 0, 1)), c));
      s1[0] = (ch_11[0] * z1[0] - (-ch_11[1]) * z1[1]) / chAvg;
      s1[1] = (ch_11[0] * z1[1] + (-ch_11[1]) * z1[0]) / chAvg;

      s1[0] += (ch_21[0] * z3[0] - (-ch_21[1]) * z3[1]) / chAvg;
      s1[1] += (ch_21[0] * z3[1] + (-ch_21[1]) * z3[0]) / chAvg;

      s1[0] += (ch_12[0] * z2[0] - ch_12[1] * -z2[1]) / chAvg;
      s1[1] += (ch_12[0] * -z2[1] + ch_12[1] * z2[0]) / chAvg;

      s1[0] += (ch_22[0] * z4[0] - ch_22[1] * -z4[1]) / chAvg;
      s1[1] += (ch_22[0] * -z4[1] + ch_22[1] * z4[0]) / chAvg;

      s1[0] = (s1[0]) * 10000;
      s1[1] = (s1[1]) * 10000;

      s1[0] = s1[0] / a[0];
      s1[1] = s1[1] / a[0];

      s1[0] = (s1[0]) * 1000;
      s1[1] = (s1[1]) * 1000;

      s1[0] = s1[0] / chPwr;
      s1[1] = s1[1] / chPwr;

      // s1[0] = (s1[0]) * chAvg;
      // s1[1] = (s1[1]) * chAvg;

      // s2 = dlsch_stbc_sub(dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_12, z1, 1, 0), dlsch_stbc_mul(ch_22, z3, 1, 0)), a), dlsch_stbc_div(dlsch_stbc_add(dlsch_stbc_mul(ch_11, z2, 0, 1), dlsch_stbc_mul(ch_21, z4, 0, 1)), c));
      s2[0] = (ch_12[0] * z1[0] - (-ch_12[1]) * z1[1]) / chAvg;
      s2[1] = (ch_12[0] * z1[1] + (-ch_12[1]) * z1[0]) / chAvg;

      s2[0] += (ch_22[0] * z3[0] - (-ch_22[1]) * z3[1]) / chAvg;
      s2[1] += (ch_22[0] * z3[1] + (-ch_22[1]) * z3[0]) / chAvg;

      s2[0] -= (ch_11[0] * z2[0] - ch_11[1] * -z2[1]) / chAvg;
      s2[1] -= (ch_11[0] * -z2[1] + ch_11[1] * z2[0]) / chAvg;

      s2[0] -= (ch_21[0] * z4[0] - ch_21[1] * -z4[1]) / chAvg;
      s2[1] -= (ch_21[0] * -z4[1] + ch_21[1] * z4[0]) / chAvg;

      s2[0] = (s2[0]) * 10000;
      s2[1] = (s2[1]) * 10000;

      s2[0] = s2[0] / a[0];
      s2[1] = s2[1] / a[0];


      s2[0] = (s2[0]) * 1000;
      s2[1] = (s2[1]) * 1000;

      s2[0] = s2[0] / chPwr;
      s2[1] = s2[1] / chPwr;


            LOG_UI(PHY, "%d - %d\n", s1[0], s1[1]);
            LOG_UI(PHY, "%d - %d\n", s2[0], s2[1]);
            LOG_UI(PHY, "%d - %d\n", s3[0], s3[1]);
            LOG_UI(PHY, "%d - %d\n", s4[0], s4[1]);
            exit(0);
      // printf("%d - %d\n", s1[0], s1[1]);

            FILE * f = fopen("TEST_QAM_DEC.txt", "w");
            fprintf(f, "%d - %d\n", s1[0], s1[1]);
            fprintf(f, "%d - %d\n", s2[0], s2[1]);
            fprintf(f, "%d - %d\n", s3[0], s3[1]);
            fprintf(f, "%d - %d\n", s4[0], s4[1]);
            // fprintf(f, "%d - %d\n", rxF0[jj], rxF0[jj + 1]);
            // fprintf(f, "%d - %d\n", rxF1[jj], rxF1[jj + 1]);
            // fprintf(f, "%d - %d\n", rxF0[jj + 2], rxF0[jj + 3]);
            // fprintf(f, "%d - %d\n", rxF1[jj + 2], rxF1[jj + 3]);
            
            fclose(f);
            exit(0);

      *rxF0_comp++ = s1[0];
      *rxF0_comp++ = s1[1];
      *rxF0_comp++ = s2[0];
      *rxF0_comp++ = s2[1];
      *rxF0_comp++ = s3[0];
      *rxF0_comp++ = s3[1];
      *rxF0_comp++ = s4[0];
      *rxF0_comp++ = s4[1];

      rxF0 += 4;
      rxF1 += 4;
      // ch_11 += 4;
      // ch_12 += 4;
      // ch_21 += 4;
      // ch_22 += 4;
      
    }
  }

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
