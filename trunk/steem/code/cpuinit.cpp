/*---------------------------------------------------------------------------
FILE: cpuinit.cpp
MODULE: emu
DESCRIPTION: Initialisation for CPU jump tables.
---------------------------------------------------------------------------*/

/* places to change when altering jump table method:
m68k_trace()
m68k_PROCESS
definitions at top of cpu.cpp
*/

void cpu_routines_init()
{
  m68k_high_nibble_jump_table[0]=m68k_0000;
  m68k_high_nibble_jump_table[1]=m68k_0001;
  m68k_high_nibble_jump_table[2]=m68k_0010;
  m68k_high_nibble_jump_table[3]=m68k_0011;
  m68k_high_nibble_jump_table[4]=m68k_0100;
  m68k_high_nibble_jump_table[5]=m68k_0101;
  m68k_high_nibble_jump_table[6]=m68k_0110;
  m68k_high_nibble_jump_table[7]=m68k_0111;
  m68k_high_nibble_jump_table[8]=m68k_1000;
  m68k_high_nibble_jump_table[9]=m68k_1001;
  m68k_high_nibble_jump_table[10]=m68k_1010;
  m68k_high_nibble_jump_table[11]=m68k_1011;
  m68k_high_nibble_jump_table[12]=m68k_1100;
  m68k_high_nibble_jump_table[13]=m68k_1101;
  m68k_high_nibble_jump_table[14]=m68k_1110;
  m68k_high_nibble_jump_table[15]=m68k_1111;

  m68k_jump_get_source_b[0]=m68k_get_source_000_b;
  m68k_jump_get_source_w[0]=m68k_get_source_000_w;
  m68k_jump_get_source_l[0]=m68k_get_source_000_l;
  m68k_jump_get_source_b[1]=m68k_get_source_001_b;
  m68k_jump_get_source_w[1]=m68k_get_source_001_w;
  m68k_jump_get_source_l[1]=m68k_get_source_001_l;
  m68k_jump_get_source_b[2]=m68k_get_source_010_b;
  m68k_jump_get_source_w[2]=m68k_get_source_010_w;
  m68k_jump_get_source_l[2]=m68k_get_source_010_l;
  m68k_jump_get_source_b[3]=m68k_get_source_011_b;
  m68k_jump_get_source_w[3]=m68k_get_source_011_w;
  m68k_jump_get_source_l[3]=m68k_get_source_011_l;
  m68k_jump_get_source_b[4]=m68k_get_source_100_b;
  m68k_jump_get_source_w[4]=m68k_get_source_100_w;
  m68k_jump_get_source_l[4]=m68k_get_source_100_l;
  m68k_jump_get_source_b[5]=m68k_get_source_101_b;
  m68k_jump_get_source_w[5]=m68k_get_source_101_w;
  m68k_jump_get_source_l[5]=m68k_get_source_101_l;
  m68k_jump_get_source_b[6]=m68k_get_source_110_b;
  m68k_jump_get_source_w[6]=m68k_get_source_110_w;
  m68k_jump_get_source_l[6]=m68k_get_source_110_l;
  m68k_jump_get_source_b[7]=m68k_get_source_111_b;
  m68k_jump_get_source_w[7]=m68k_get_source_111_w;
  m68k_jump_get_source_l[7]=m68k_get_source_111_l;

  for(int n=0;n<8;n++){
    m68k_jump_get_source_b_not_a[n]=m68k_jump_get_source_b[n];
    m68k_jump_get_source_w_not_a[n]=m68k_jump_get_source_w[n];
    m68k_jump_get_source_l_not_a[n]=m68k_jump_get_source_l[n];
  }
  m68k_jump_get_source_b_not_a[1]=m68k_unrecognised;
  m68k_jump_get_source_w_not_a[1]=m68k_unrecognised;
  m68k_jump_get_source_l_not_a[1]=m68k_unrecognised;


  m68k_jump_get_dest_b[0]=m68k_get_dest_000_b;
  m68k_jump_get_dest_w[0]=m68k_get_dest_000_w;
  m68k_jump_get_dest_l[0]=m68k_get_dest_000_l;
  m68k_jump_get_dest_b[1]=m68k_get_dest_001_b;
  m68k_jump_get_dest_w[1]=m68k_get_dest_001_w;
  m68k_jump_get_dest_l[1]=m68k_get_dest_001_l;
  m68k_jump_get_dest_b[2]=m68k_get_dest_010_b;
  m68k_jump_get_dest_w[2]=m68k_get_dest_010_w;
  m68k_jump_get_dest_l[2]=m68k_get_dest_010_l;
  m68k_jump_get_dest_b[3]=m68k_get_dest_011_b;
  m68k_jump_get_dest_w[3]=m68k_get_dest_011_w;
  m68k_jump_get_dest_l[3]=m68k_get_dest_011_l;
  m68k_jump_get_dest_b[4]=m68k_get_dest_100_b;
  m68k_jump_get_dest_w[4]=m68k_get_dest_100_w;
  m68k_jump_get_dest_l[4]=m68k_get_dest_100_l;
  m68k_jump_get_dest_b[5]=m68k_get_dest_101_b;
  m68k_jump_get_dest_w[5]=m68k_get_dest_101_w;
  m68k_jump_get_dest_l[5]=m68k_get_dest_101_l;
  m68k_jump_get_dest_b[6]=m68k_get_dest_110_b;
  m68k_jump_get_dest_w[6]=m68k_get_dest_110_w;
  m68k_jump_get_dest_l[6]=m68k_get_dest_110_l;
  m68k_jump_get_dest_b[7]=m68k_get_dest_111_b;
  m68k_jump_get_dest_w[7]=m68k_get_dest_111_w;
  m68k_jump_get_dest_l[7]=m68k_get_dest_111_l;

  for(int n=0;n<8;n++){
    m68k_jump_get_dest_b_not_a[n]=m68k_jump_get_dest_b[n];
    m68k_jump_get_dest_w_not_a[n]=m68k_jump_get_dest_w[n];
    m68k_jump_get_dest_l_not_a[n]=m68k_jump_get_dest_l[n];
  }
  m68k_jump_get_dest_b_not_a[1]=m68k_unrecognised;
  m68k_jump_get_dest_w_not_a[1]=m68k_unrecognised;
  m68k_jump_get_dest_l_not_a[1]=m68k_unrecognised;

  for(int n=0;n<8;n++){
    m68k_jump_get_dest_b_not_a_or_d[n]=m68k_jump_get_dest_b_not_a[n];
    m68k_jump_get_dest_w_not_a_or_d[n]=m68k_jump_get_dest_w_not_a[n];
    m68k_jump_get_dest_l_not_a_or_d[n]=m68k_jump_get_dest_l_not_a[n];
    m68k_jump_get_dest_b_not_a_faster_for_d[n]=m68k_jump_get_dest_b_not_a[n];
    m68k_jump_get_dest_w_not_a_faster_for_d[n]=m68k_jump_get_dest_w_not_a[n];
    m68k_jump_get_dest_l_not_a_faster_for_d[n]=m68k_jump_get_dest_l_not_a[n];
  }
  m68k_jump_get_dest_b_not_a_or_d[0]=m68k_unrecognised;
  m68k_jump_get_dest_w_not_a_or_d[0]=m68k_unrecognised;
  m68k_jump_get_dest_l_not_a_or_d[0]=m68k_unrecognised;

  m68k_jump_get_dest_b_not_a_faster_for_d[0]=m68k_get_dest_000_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[0]=m68k_get_dest_000_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[0]=m68k_get_dest_000_l_faster;
  m68k_jump_get_dest_b_not_a_faster_for_d[1]=m68k_unrecognised;
  m68k_jump_get_dest_w_not_a_faster_for_d[1]=m68k_unrecognised;
  m68k_jump_get_dest_l_not_a_faster_for_d[1]=m68k_unrecognised;
  m68k_jump_get_dest_b_not_a_faster_for_d[2]=m68k_get_dest_010_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[2]=m68k_get_dest_010_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[2]=m68k_get_dest_010_l_faster;
  m68k_jump_get_dest_b_not_a_faster_for_d[3]=m68k_get_dest_011_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[3]=m68k_get_dest_011_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[3]=m68k_get_dest_011_l_faster;
  m68k_jump_get_dest_b_not_a_faster_for_d[4]=m68k_get_dest_100_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[4]=m68k_get_dest_100_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[4]=m68k_get_dest_100_l_faster;
  m68k_jump_get_dest_b_not_a_faster_for_d[5]=m68k_get_dest_101_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[5]=m68k_get_dest_101_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[5]=m68k_get_dest_101_l_faster;
  m68k_jump_get_dest_b_not_a_faster_for_d[6]=m68k_get_dest_110_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[6]=m68k_get_dest_110_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[6]=m68k_get_dest_110_l_faster;
  m68k_jump_get_dest_b_not_a_faster_for_d[7]=m68k_get_dest_111_b_faster;
  m68k_jump_get_dest_w_not_a_faster_for_d[7]=m68k_get_dest_111_w_faster;
  m68k_jump_get_dest_l_not_a_faster_for_d[7]=m68k_get_dest_111_l_faster;

/*
  for(int n=0;n<8;n++){
    m68k_jump_get_dest_b_not_a_faster_for_d[n]=m68k_jump_get_dest_b_not_a[n];
    m68k_jump_get_dest_w_not_a_faster_for_d[n]=m68k_jump_get_dest_w_not_a[n];
    m68k_jump_get_dest_l_not_a_faster_for_d[n]=m68k_jump_get_dest_l_not_a[n];
  }
*/

  for(int n=0;n<64;n++){
    m68k_jump_line_0[n]=m68k_unrecognised;
    m68k_jump_line_4[n]=m68k_unrecognised;
    m68k_jump_line_4_stuff[n]=m68k_unrecognised;
    m68k_jump_line_e[n]=m68k_unrecognised;
  }

  m68k_jump_line_0[B6_000000]=m68k_ori_b;
  m68k_jump_line_0[B6_000001]=m68k_ori_w;
  m68k_jump_line_0[B6_000010]=m68k_ori_l;
  m68k_jump_line_0[B6_001000]=m68k_andi_b;
  m68k_jump_line_0[B6_001001]=m68k_andi_w;
  m68k_jump_line_0[B6_001010]=m68k_andi_l;
  m68k_jump_line_0[B6_010000]=m68k_subi_b;
  m68k_jump_line_0[B6_010001]=m68k_subi_w;
  m68k_jump_line_0[B6_010010]=m68k_subi_l;
  m68k_jump_line_0[B6_011000]=m68k_addi_b;
  m68k_jump_line_0[B6_011001]=m68k_addi_w;
  m68k_jump_line_0[B6_011010]=m68k_addi_l;
  m68k_jump_line_0[B6_100000]=m68k_btst;
  m68k_jump_line_0[B6_100001]=m68k_bchg;
  m68k_jump_line_0[B6_100010]=m68k_bclr;
  m68k_jump_line_0[B6_100011]=m68k_bset;
  m68k_jump_line_0[B6_101000]=m68k_eori_b;
  m68k_jump_line_0[B6_101001]=m68k_eori_w;
  m68k_jump_line_0[B6_101010]=m68k_eori_l;
  m68k_jump_line_0[B6_110000]=m68k_cmpi_b;
  m68k_jump_line_0[B6_110001]=m68k_cmpi_w;
  m68k_jump_line_0[B6_110010]=m68k_cmpi_l;
  for(int a=B6_000000;a<=B6_111000;a+=B6_001000){
    m68k_jump_line_0[B6_000100+a]=m68k_movep_w_to_dN_or_btst;
    m68k_jump_line_0[B6_000101+a]=m68k_movep_l_to_dN_or_bchg;
    m68k_jump_line_0[B6_000110+a]=m68k_movep_w_from_dN_or_bclr;
    m68k_jump_line_0[B6_000111+a]=m68k_movep_l_from_dN_or_bset;
  }

  m68k_jump_line_4[B6_000000]=m68k_negx_b;
  m68k_jump_line_4[B6_000001]=m68k_negx_w;
  m68k_jump_line_4[B6_000010]=m68k_negx_l;
  m68k_jump_line_4[B6_000011]=m68k_move_from_sr;
  m68k_jump_line_4[B6_001000]=m68k_clr_b;
  m68k_jump_line_4[B6_001001]=m68k_clr_w;
  m68k_jump_line_4[B6_001010]=m68k_clr_l;
  m68k_jump_line_4[B6_001011]=m68k_move_from_ccr;
  m68k_jump_line_4[B6_010000]=m68k_neg_b;
  m68k_jump_line_4[B6_010001]=m68k_neg_w;
  m68k_jump_line_4[B6_010010]=m68k_neg_l;
  m68k_jump_line_4[B6_010011]=m68k_move_to_ccr;
  m68k_jump_line_4[B6_011000]=m68k_not_b;
  m68k_jump_line_4[B6_011001]=m68k_not_w;
  m68k_jump_line_4[B6_011010]=m68k_not_l;
  m68k_jump_line_4[B6_011011]=m68k_move_to_sr;
  m68k_jump_line_4[B6_100000]=m68k_nbcd;
  m68k_jump_line_4[B6_100001]=m68k_pea_or_swap;
  m68k_jump_line_4[B6_100010]=m68k_movem_w_from_regs_or_ext_w;
  m68k_jump_line_4[B6_100011]=m68k_movem_l_from_regs_or_ext_l;
  m68k_jump_line_4[B6_101000]=m68k_tst_b;
  m68k_jump_line_4[B6_101001]=m68k_tst_w;
  m68k_jump_line_4[B6_101010]=m68k_tst_l;
  m68k_jump_line_4[B6_101011]=m68k_tas;
  m68k_jump_line_4[B6_110010]=m68k_movem_w_to_regs;
  m68k_jump_line_4[B6_110011]=m68k_movem_l_to_regs;
  m68k_jump_line_4[B6_111001]=m68k_line_4_stuff;
  m68k_jump_line_4[B6_111010]=m68k_jsr;
  m68k_jump_line_4[B6_111011]=m68k_jmp;

  for(int n=0;n<8;n++){
    m68k_jump_line_4[(n<<3)+6]=m68k_chk;
    m68k_jump_line_4[(n<<3)+7]=m68k_lea;
    m68k_jump_line_4_stuff[BITS_543_000+n]=m68k_trap;
    m68k_jump_line_4_stuff[BITS_543_001+n]=m68k_trap;
    m68k_jump_line_4_stuff[BITS_543_010+n]=m68k_link;
    m68k_jump_line_4_stuff[BITS_543_011+n]=m68k_unlk;
    m68k_jump_line_4_stuff[BITS_543_100+n]=m68k_move_to_usp;
    m68k_jump_line_4_stuff[BITS_543_101+n]=m68k_move_from_usp;
  }

  m68k_jump_line_4_stuff[B6_110000]=m68k_reset;
  m68k_jump_line_4_stuff[B6_110001]=m68k_nop;
  m68k_jump_line_4_stuff[B6_110010]=m68k_stop;
  m68k_jump_line_4_stuff[B6_110011]=m68k_rte;
  m68k_jump_line_4_stuff[B6_110100]=m68k_rtd;
  m68k_jump_line_4_stuff[B6_110101]=m68k_rts;
  m68k_jump_line_4_stuff[B6_110110]=m68k_trapv;
  m68k_jump_line_4_stuff[B6_110111]=m68k_rtr;
  m68k_jump_line_4_stuff[B6_111010]=m68k_movec;
  m68k_jump_line_4_stuff[B6_111011]=m68k_movec;

  m68k_jump_line_5[0]=m68k_addq_b;
  m68k_jump_line_5[1]=m68k_addq_w;
  m68k_jump_line_5[2]=m68k_addq_l;
  m68k_jump_line_5[3]=m68k_dbCC_or_sCC;
  m68k_jump_line_5[4]=m68k_subq_b;
  m68k_jump_line_5[5]=m68k_subq_w;
  m68k_jump_line_5[6]=m68k_subq_l;
  m68k_jump_line_5[7]=m68k_dbCC_or_sCC;

  m68k_jump_line_8[0]=m68k_or_b_to_dN;
  m68k_jump_line_8[1]=m68k_or_w_to_dN;
  m68k_jump_line_8[2]=m68k_or_l_to_dN;
  m68k_jump_line_8[3]=m68k_divu;
  m68k_jump_line_8[4]=m68k_or_b_from_dN_or_sbcd;
  m68k_jump_line_8[5]=m68k_or_w_from_dN;
  m68k_jump_line_8[6]=m68k_or_l_from_dN;
  m68k_jump_line_8[7]=m68k_divs;

  m68k_jump_line_9[0]=m68k_sub_b_to_dN;
  m68k_jump_line_9[1]=m68k_sub_w_to_dN;
  m68k_jump_line_9[2]=m68k_sub_l_to_dN;
  m68k_jump_line_9[3]=m68k_suba_w;
  m68k_jump_line_9[4]=m68k_sub_b_from_dN;
  m68k_jump_line_9[5]=m68k_sub_w_from_dN;
  m68k_jump_line_9[6]=m68k_sub_l_from_dN;
  m68k_jump_line_9[7]=m68k_suba_l;

  m68k_jump_line_b[0]=m68k_cmp_b;
  m68k_jump_line_b[1]=m68k_cmp_w;
  m68k_jump_line_b[2]=m68k_cmp_l;
  m68k_jump_line_b[3]=m68k_cmpa_w;
  m68k_jump_line_b[4]=m68k_eor_b;
  m68k_jump_line_b[5]=m68k_eor_w;
  m68k_jump_line_b[6]=m68k_eor_l;
  m68k_jump_line_b[7]=m68k_cmpa_l;

  m68k_jump_line_c[0]=m68k_and_b_to_dN;
  m68k_jump_line_c[1]=m68k_and_w_to_dN;
  m68k_jump_line_c[2]=m68k_and_l_to_dN;
  m68k_jump_line_c[3]=m68k_mulu;
  m68k_jump_line_c[4]=m68k_and_b_from_dN_or_abcd;
  m68k_jump_line_c[5]=m68k_and_w_from_dN_or_exg_like;
  m68k_jump_line_c[6]=m68k_and_l_from_dN_or_exg_unlike;
  m68k_jump_line_c[7]=m68k_muls;

  m68k_jump_line_d[0]=m68k_add_b_to_dN;
  m68k_jump_line_d[1]=m68k_add_w_to_dN;
  m68k_jump_line_d[2]=m68k_add_l_to_dN;
  m68k_jump_line_d[3]=m68k_adda_w;
  m68k_jump_line_d[4]=m68k_add_b_from_dN;
  m68k_jump_line_d[5]=m68k_add_w_from_dN;
  m68k_jump_line_d[6]=m68k_add_l_from_dN;
  m68k_jump_line_d[7]=m68k_adda_l;

  m68k_jump_line_e[B6_000000]=m68k_asr_b_to_dM;
  m68k_jump_line_e[B6_000001]=m68k_lsr_b_to_dM;
  m68k_jump_line_e[B6_000010]=m68k_roxr_b_to_dM;
  m68k_jump_line_e[B6_000011]=m68k_ror_b_to_dM;
  m68k_jump_line_e[B6_001000]=m68k_asr_w_to_dM;
  m68k_jump_line_e[B6_001001]=m68k_lsr_w_to_dM;
  m68k_jump_line_e[B6_001010]=m68k_roxr_w_to_dM;
  m68k_jump_line_e[B6_001011]=m68k_ror_w_to_dM;
  m68k_jump_line_e[B6_010000]=m68k_asr_l_to_dM;
  m68k_jump_line_e[B6_010001]=m68k_lsr_l_to_dM;
  m68k_jump_line_e[B6_010010]=m68k_roxr_l_to_dM;
  m68k_jump_line_e[B6_010011]=m68k_ror_l_to_dM;
  m68k_jump_line_e[B6_100000]=m68k_asl_b_to_dM;
  m68k_jump_line_e[B6_100001]=m68k_lsl_b_to_dM;
  m68k_jump_line_e[B6_100010]=m68k_roxl_b_to_dM;
  m68k_jump_line_e[B6_100011]=m68k_rol_b_to_dM;
  m68k_jump_line_e[B6_101000]=m68k_asl_w_to_dM;
  m68k_jump_line_e[B6_101001]=m68k_lsl_w_to_dM;
  m68k_jump_line_e[B6_101010]=m68k_roxl_w_to_dM;
  m68k_jump_line_e[B6_101011]=m68k_rol_w_to_dM;
  m68k_jump_line_e[B6_110000]=m68k_asl_l_to_dM;
  m68k_jump_line_e[B6_110001]=m68k_lsl_l_to_dM;
  m68k_jump_line_e[B6_110010]=m68k_roxl_l_to_dM;
  m68k_jump_line_e[B6_110011]=m68k_rol_l_to_dM;


  m68k_jump_condition_test[0]=m68k_condition_test_t;
  m68k_jump_condition_test[1]=m68k_condition_test_f;
  m68k_jump_condition_test[2]=m68k_condition_test_hi;
  m68k_jump_condition_test[3]=m68k_condition_test_ls;
  m68k_jump_condition_test[4]=m68k_condition_test_cc;
  m68k_jump_condition_test[5]=m68k_condition_test_cs;
  m68k_jump_condition_test[6]=m68k_condition_test_ne;
  m68k_jump_condition_test[7]=m68k_condition_test_eq;
  m68k_jump_condition_test[8]=m68k_condition_test_vc;
  m68k_jump_condition_test[9]=m68k_condition_test_vs;
  m68k_jump_condition_test[10]=m68k_condition_test_pl;
  m68k_jump_condition_test[11]=m68k_condition_test_mi;
  m68k_jump_condition_test[12]=m68k_condition_test_ge;
  m68k_jump_condition_test[13]=m68k_condition_test_lt;
  m68k_jump_condition_test[14]=m68k_condition_test_gt;
  m68k_jump_condition_test[15]=m68k_condition_test_le;

  for(int a=B6_000000;a<=B6_111000;a+=B6_001000){
    for(int b=B6_000000;b<=B6_000011;b+=B6_000001){
      m68k_jump_line_e[a+b+B6_000100]=m68k_jump_line_e[a+b]; //for bit shifting to data registers, immediate mode and dN, mode point to same function
    }
  }
  for(int n=2;n<8;n++){
    m68k_jump_line_e[B6_011000+n]=m68k_bit_shift_right_to_mem;
    m68k_jump_line_e[B6_111000+n]=m68k_bit_shift_left_to_mem;
  }


}

