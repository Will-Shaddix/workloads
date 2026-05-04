#ifdef OMB_NAME_osu_latency_persistent
#define OMB_NAME    "osu_latency_persistent"
#define OMB_SUBTYPE LAT
#define BENCHMARK   "OSU MPI%s Latency Test Persistent"
#endif

#ifdef OMB_NAME_osu_bw_persistent
#define OMB_NAME    "osu_bw_persistent"
#define OMB_SUBTYPE BW
#define BENCHMARK   "OSU MPI%s Bandwidth Test Persistent"
#endif

#ifdef OMB_NAME_osu_bibw_persistent
#define OMB_NAME    "osu_bibw_persistent"
#define OMB_SUBTYPE BI_BW
#define BENCHMARK   "OSU MPI%s Bi-Directional Bandwidth Test Persistent"
#endif
