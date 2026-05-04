#ifdef OMB_NAME_osu_bw_fan_in
#define OMB_NAME    "osu_bw_fan_in"
#define OMB_SUBTYPE CONG_BW
#define BENCHMARK   "OSU MPI%s Bandwidth Congestion In Test"
#define OMB_FAN_OUT 0
#endif

#ifdef OMB_NAME_osu_bw_fan_out
#define OMB_NAME    "osu_bw_fan_out"
#define OMB_SUBTYPE CONG_BW
#define BENCHMARK   "OSU MPI%s Bandwidth Congestion Out Test"
#define OMB_FAN_OUT 1
#endif
