#ifdef OMB_NAME_osu_latency
#define OMB_NAME    "osu_latency"
#define OMB_SUBTYPE LAT
#define BENCHMARK   "OSU MPI%s Latency Test Test"
#endif

#ifdef OMB_NAME_osu_latency_mp
#define OMB_NAME    "osu_latency_mp"
#define OMB_SUBTYPE LAT_MP
#define BENCHMARK   "OSU MPI%s Multi-process Latency Test"

#ifndef _ENABLE_MPI2_
#define OMB_DONT_BUILD_ENTRY 1
#define OMB_DONT_BUILD_MSG   "This test requires _ENABLE_MPI2_\n"
#endif
#endif

#ifdef OMB_NAME_osu_latency_mt
#define OMB_NAME    "osu_latency_mt"
#define OMB_SUBTYPE LAT_MT
#define BENCHMARK   "OSU MPI%s Multi-threaded Latency Test"

#ifndef _ENABLE_MPI2_
#define OMB_DONT_BUILD_ENTRY 1
#define OMB_DONT_BUILD_MSG   "This test requires _ENABLE_MPI2_\n"
#endif
#endif

#ifdef OMB_NAME_osu_bw
#define OMB_NAME    "osu_bw"
#define OMB_SUBTYPE BW
#define BENCHMARK   "OSU MPI%s Bandwidth Test"
#endif

#ifdef OMB_NAME_osu_bibw
#define OMB_NAME    "osu_bibw"
#define OMB_SUBTYPE BI_BW
#define BENCHMARK   "OSU MPI%s Bi-Directional Bandwidth Test"
#endif

#ifdef OMB_NAME_osu_mbw_mr
#define OMB_NAME    "osu_mbw_mr"
#define OMB_SUBTYPE MBW_MR
#define BENCHMARK   "OSU MPI%s Multiple Bandwidth / Message Rate Test"
#define OMB_PAIRED  1
#endif

#ifdef OMB_NAME_osu_multi_lat
#define OMB_NAME    "osu_multi_lat"
#define OMB_SUBTYPE LAT
#define BENCHMARK   "OSU MPI%s Multi Latency Test Test"
#define OMB_PAIRED  1
#endif

#ifdef OMB_NAME_osu_partitioned_latency
#define OMB_NAME       "osu_partitioned_latency"
#define OMB_SUBTYPE    PART_LAT
#define BENCHMARK      "OSU MPI%s Partitioned Latency Test"
#define OMB_PERSISTENT 1

#ifndef _MPI4_PARTITION_PT2PT_
#define OMB_DONT_BUILD_ENTRY 1
#define OMB_DONT_BUILD_MSG   "This test requires _MPI4_PARTITION_PT2PT_\n"
#endif
#endif
