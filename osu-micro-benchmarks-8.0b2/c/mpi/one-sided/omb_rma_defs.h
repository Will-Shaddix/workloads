#ifdef OMB_NAME_osu_get_bw
#define OMB_NAME    "osu_get_bw"
#define OMB_SUBTYPE BW
#define OMB_MPI_OP  GET
#define BENCHMARK   "OSU MPI_Get%s Bandwidth Test"
#endif

#ifdef OMB_NAME_osu_get_latency
#define OMB_NAME    "osu_get_latency"
#define OMB_SUBTYPE LAT
#define OMB_MPI_OP  GET
#define BENCHMARK   "OSU MPI_Get%s latency Test"
#endif

#ifdef OMB_NAME_osu_put_bibw
#define OMB_NAME    "osu_put_bibw"
#define OMB_SUBTYPE BI_BW
#define OMB_MPI_OP  PUT
#define BENCHMARK   "OSU MPI_Put%s Bi-directional Bandwidth Test"
#endif

#ifdef OMB_NAME_osu_put_bw
#define OMB_NAME    "osu_put_bw"
#define OMB_SUBTYPE BW
#define OMB_MPI_OP  PUT
#define BENCHMARK   "OSU MPI_Put%s Bandwidth Test"
#endif

#ifdef OMB_NAME_osu_put_latency
#define OMB_NAME    "osu_put_latency"
#define OMB_SUBTYPE LAT
#define OMB_MPI_OP  PUT
#define BENCHMARK   "OSU MPI_Put%s Latency Test"
#endif

#ifdef OMB_NAME_osu_acc_latency
#define OMB_NAME    "osu_acc_latency"
#define OMB_SUBTYPE LAT
#define OMB_MPI_OP  ACCUMULATE
#define BENCHMARK   "OSU MPI_Accumulate%s latency Test"
#endif

#ifdef OMB_NAME_osu_cas_latency
#define OMB_NAME    "osu_cas_latency"
#define OMB_SUBTYPE LAT
#define OMB_MPI_OP  COMPARE_AND_SWAP
#define BENCHMARK   "OSU MPI_Compare_and_swap%s latency Test"
#endif

#ifdef OMB_NAME_osu_fop_latency
#define OMB_NAME    "osu_fop_latency"
#define OMB_SUBTYPE LAT
#define OMB_MPI_OP  FETCH_AND_OP
#define BENCHMARK   "OSU MPI_Fetch_and_op%s latency Test"
#endif

#ifdef OMB_NAME_osu_get_acc_latency
#define OMB_NAME    "osu_get_acc_latency"
#define OMB_SUBTYPE LAT
#define OMB_MPI_OP  GET_ACCUMULATE
#define BENCHMARK   "OSU MPI_Get_accumulate latency Test"
#endif
