
#include "status/status.h"
#include "perf/perf.h"

typedef struct Status Status_t, *StatusP_t;

struct Status
{
    String_t szHeader;
    String_t szReference;
    PerfDataItemP_t pItem;
};

GPrivate Status_t gTicks = {"Timestamp", "sun.os.hrt.ticks", NULL };
GPrivate Status_t gFrequency = {"Timestamp", "sun.os.hrt.frequency", NULL };

GPrivate Status_t gS0C = { "S0C", "sun.gc.generation.0.space.1.capacity", NULL };
GPrivate Status_t gS1C = { "S1C", "sun.gc.generation.0.space.2.capacity", NULL };

GPrivate Status_t gS0U = { "S0U", "sun.gc.generation.0.space.1.used", NULL };
GPrivate Status_t gS1U = { "S1U", "sun.gc.generation.0.space.2.used", NULL };

GPrivate Status_t gEC = { "EC", "sun.gc.generation.0.space.0.capacity", NULL };
GPrivate Status_t gEU = { "EU", "sun.gc.generation.0.space.0.used", NULL };

GPrivate Status_t gOC = { "OC", "sun.gc.generation.1.space.0.capacity", NULL };
GPrivate Status_t gOU = { "OU", "sun.gc.generation.1.space.0.used", NULL };

GPrivate Status_t gPC = { "PC", "sun.gc.generation.2.space.0.capacity", NULL };
GPrivate Status_t gPU = { "PU", "sun.gc.generation.2.space.0.used", NULL };

GPrivate Status_t gNGCMN = { "NGCMN", "sun.gc.generation.0.minCapacity", NULL };
GPrivate Status_t gNGCMX = { "NGCMX", "sun.gc.generation.0.maxCapacity", NULL };
GPrivate Status_t gNGC = { "NGC", "sun.gc.generation.0.capacity", NULL };

GPrivate Status_t gOGCMN = { "OGCMN", "sun.gc.generation.1.minCapacity", NULL };
GPrivate Status_t gOGCMX = { "OGCMX", "sun.gc.generation.1.maxCapacity", NULL };
GPrivate Status_t gOGC = { "OGC", "sun.gc.generation.1.capacity", NULL };

GPrivate Status_t gPGCMN = { "PGCMN", "sun.gc.generation.2.minCapacity", NULL };
GPrivate Status_t gPGCMX = { "PGCMX", "sun.gc.generation.2.maxCapacity", NULL };
GPrivate Status_t gPGC = { "PGC", "sun.gc.generation.2.capacity", NULL };

GPrivate Status_t gYGC = { "YGC", "sun.gc.collector.0.invocations", NULL };
GPrivate Status_t gFGC = { "FGC", "sun.gc.collector.1.invocations", NULL };

GPrivate Status_t gYGCT = { "YGCT", "sun.gc.collector.0.time", NULL };
GPrivate Status_t gFGCT = { "FGCT", "sun.gc.collector.1.time", NULL };

GPrivate Double_t gdfPrevYGCT = 0.000;
GPrivate Double_t gdfPrevFGCT = 0.000;

#define S_ITEM(s) s.pItem = pdi_search_item(pTree, s.szReference)
GPublic void status_init(RBTreeP_t pTree)
{
    S_ITEM(gTicks);
    S_ITEM(gFrequency);
    S_ITEM(gS0C);
    S_ITEM(gS1C);
    S_ITEM(gS0U);
    S_ITEM(gS1U);
    S_ITEM(gEC);
    S_ITEM(gEU);
    S_ITEM(gOC);
    S_ITEM(gOU);
    S_ITEM(gPC);
    S_ITEM(gPU);
    S_ITEM(gNGCMN);
    S_ITEM(gNGCMX);
    S_ITEM(gNGC);
    S_ITEM(gOGCMN);
    S_ITEM(gOGCMX);
    S_ITEM(gOGC);
    S_ITEM(gPGCMN);
    S_ITEM(gPGCMX);
    S_ITEM(gPGC);
    S_ITEM(gYGC);
    S_ITEM(gFGC);
    S_ITEM(gYGCT);
    S_ITEM(gFGCT);
}

#define S_JLONG(s) pdi_get_jlong(s.pItem)
#define S_DOUBLE(s) pdi_get_double(s.pItem)

#define S_J2D(j) ((Double_t)(j))
#define S_JLONG2D(s) S_J2D(S_JLONG(s))

GPrivate jlong s_ticks()
{
    return S_JLONG(gTicks);
}

GPrivate jlong s_frequency()
{
    return S_JLONG(gFrequency);
}

GPrivate Double_t s_timstamp()
{
    return (Double_t)(S_J2D(s_ticks()) / S_J2D(s_frequency()));
}

GPrivate jlong s_s0c()
{
    return S_JLONG(gS0C) / 1024;
}

GPrivate jlong s_s1c()
{
    return S_JLONG(gS1C) / 1024;
}

GPrivate jlong s_s0u()
{
    return S_JLONG(gS0U) / 1024;
}

GPrivate jlong s_s1u()
{
    return S_JLONG(gS1U) / 1024;
}

GPrivate jlong s_ec()
{
    return S_JLONG(gEC) / 1024;
}

GPrivate jlong s_eu()
{
    jlong eu = S_JLONG(gEU) / 1024;
    return eu;
}

GPrivate jlong s_oc()
{
    return S_JLONG(gOC) / 1024;
}

GPrivate jlong s_ou()
{
    return S_JLONG(gOU) / 1024;
}

GPrivate jlong s_pc()
{
    return S_JLONG(gPC) / 1024;
}

GPrivate jlong s_pu()
{
    return S_JLONG(gPU) / 1024;
}

GPrivate Double_t s_s0p()
{
    return (Double_t)((1.000 - S_J2D(((S_J2D(s_s0c()) - S_J2D(s_s0u())) / S_J2D(s_s0c())))) * 100.000);
}

GPrivate Double_t s_s1p()
{
    return (Double_t)((1.000 - S_J2D(((S_J2D(s_s1c()) - S_J2D(s_s1u())) / S_J2D(s_s1c())))) * 100.000);
}

GPrivate Double_t s_ep()
{
    return (Double_t)((1.000 - S_J2D(((S_J2D(s_ec()) - S_J2D(s_eu())) / S_J2D(s_ec())))) * 100.000);
}

GPrivate Double_t s_op()
{
    return (Double_t)((1.000 - S_J2D(((S_J2D(s_oc()) - S_J2D(s_ou())) / S_J2D(s_oc())))) * 100.000);
}

GPrivate Double_t s_pp()
{
    return (Double_t)((1.000 - S_J2D(((S_J2D(s_pc()) - S_J2D(s_pu())) / S_J2D(s_pc())))) * 100.000);
}

GPrivate jlong s_ygc()
{
    return S_JLONG(gYGC);
}

GPrivate jlong s_fgc()
{
    return S_JLONG(gFGC);
}

GPrivate Double_t s_ygct()
{
    return (Double_t)(S_JLONG2D(gYGCT) / S_J2D(s_frequency()));
}

GPrivate Double_t s_aygct()
{
    return s_ygct() / S_J2D(s_ygc());
}

GPrivate Double_t s_cygct()
{
    Double_t dfCurrYGCT = s_ygct();
    Double_t dfRet = dfCurrYGCT - gdfPrevYGCT;
    gdfPrevYGCT = dfCurrYGCT;
    return dfRet;
}

GPrivate Double_t s_fgct()
{
    return (Double_t)(S_JLONG2D(gFGCT) / S_J2D(s_frequency()));
}

GPrivate Double_t s_afgct()
{
    return s_fgct() / S_J2D(s_fgc());
}

GPrivate Double_t s_cfgct()
{
    Double_t dfCurrFGCT = s_fgct();
    Double_t dfRet = dfCurrFGCT - gdfPrevFGCT;
    gdfPrevFGCT = dfCurrFGCT;
    return dfRet;
}

GPrivate Double_t s_gct()
{
    return (Double_t)((S_JLONG2D(gYGCT) + S_JLONG2D(gFGCT)) / S_J2D(s_frequency()));
}

GPrivate jlong s_ngcmn()
{
    return S_JLONG(gNGCMN) / 1024;
}

GPrivate jlong s_ngcmx()
{
    return S_JLONG(gNGCMX) / 1024;
}

GPrivate jlong s_ngc()
{
    return S_JLONG(gNGC) / 1024;
}

GPrivate jlong s_ogcmn()
{
    return S_JLONG(gOGCMN) / 1024;
}

GPrivate jlong s_ogcmx()
{
    return S_JLONG(gOGCMX) / 1024;
}

GPrivate jlong s_ogc()
{
    return S_JLONG(gOGC) / 1024;
}

GPrivate jlong s_pgcmn()
{
    return S_JLONG(gPGCMN) / 1024;
}

GPrivate jlong s_pgcmx()
{
    return S_JLONG(gPGCMX) / 1024;
}

GPrivate jlong s_pgc()
{
    return S_JLONG(gPGC) / 1024;
}

#define s_printf gcmon_debug_msg

#define S_F "%.3f"
#define S_L "%lld"

GPrivate void s_out(String_t szContext)
{
    s_printf(szContext);
    s_printf("Timestamp: "S_F"\t", s_timstamp());
    s_printf("S0C: "S_L"\t", s_s0c());
    s_printf("S1C: "S_L"\t", s_s1c());
    s_printf("S0U: "S_L"\t", s_s0u());
    s_printf("S1U: "S_L"\t", s_s1u());
    s_printf("EC: "S_L"\t", s_ec());
    s_printf("EU: "S_L"\t", s_eu());
    s_printf("OC: "S_L"\t", s_oc());
    s_printf("OU: "S_L"\t", s_ou());
    s_printf("PC: "S_L"\t", s_pc());
    s_printf("PU: "S_L"\t", s_pu());
    s_printf("S0P: "S_F"\t", s_s0p());
    s_printf("S1P: "S_F"\t", s_s1p());
    s_printf("EP: "S_F"\t", s_ep());
    s_printf("OP: "S_F"\t", s_op());
    s_printf("PP: "S_F"\t", s_pp());
    s_printf("YGC: "S_L"\t", s_ygc());
    s_printf("FGC: "S_L"\t", s_fgc());
    s_printf("YGCT: "S_F"\t", s_ygct());
    s_printf("AYGCT: "S_F"\t", s_aygct());
    //s_printf("CYGCT: "S_F"\t", s_cygct());
    s_printf("FGCT: "S_F"\t", s_fgct());
    s_printf("AFGCT: "S_F"\t", s_afgct());
    //s_printf("CFGCT: "S_F"\t", s_cfgct());
    s_printf("GCT: "S_F"\t", s_gct());
    s_printf("NGCMIN: "S_L"\t", s_ngcmn());
    s_printf("NGCMAX: "S_L"\t", s_ngcmx());
    s_printf("NGC: "S_L"\t", s_ngc());
    s_printf("OGCMIN: "S_L"\t", s_ogcmn());
    s_printf("OGCMAX: "S_L"\t", s_ogcmx());
    s_printf("OGC: "S_L"\t", s_ogc());
    s_printf("PGCMIN: "S_L"\t", s_pgcmn());
    s_printf("PGCMAX: "S_L"\t", s_pgcmx());
    s_printf("PGC: "S_L"\t", s_pgc());
    s_printf("\n");
}

GPublic void status_sample(String_t szContext)
{
    s_out(szContext);
}
