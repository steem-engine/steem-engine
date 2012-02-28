#ifdef IN_MAIN
#define EXT
#define INIT(s) =s
#else
#define EXT extern
#define INIT(s)
#endif

EXT void hdimg_init_vectors();
EXT void hdimg_reset();
EXT void hdimg_intercept(MEM_ADDRESS);
EXT void hdimg_intercept_init();

EXT bool hdimg_active INIT(0);

EXT MEM_ADDRESS os_hdimg_init_vector INIT(0),os_hdimg_bpb_vector INIT(0),os_hdimg_rw_vector INIT(0),
              os_hdimg_boot_vector INIT(0),os_hdimg_mediach_vector INIT(0);

#undef EXT
#undef INIT
