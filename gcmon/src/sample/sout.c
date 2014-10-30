
#include "sample/sample.h"
#include "os/os.h"
#include "file/file.h"
#include "buff/buff.h"
#include "gcmon/gcmon.h"

#define S_FD 0       //!< Double_t
#define S_FL 1       //!< jlong
#define S_FS 2       //!< separator

struct Souter
{
    BufferP_t bf1;
    BufferP_t bf2;
    BufferP_t cur;

    Uint32_t size;
    Uint32_t reserved;
};

#define SOUTER_SIZE sizeof(Souter_t)

GPublic SouterP_t so_new(Uint32_t size, Uint32_t reserved)
{
    SouterP_t pOuter = NULL;
    BufferP_t bf1 = NULL;
    BufferP_t bf2 = NULL;

    GMALLOC(pOuter, Souter_t, SOUTER_SIZE);
    GCMON_CHECK_NULL(pOuter, ERROR);

    bf1 = bf_new(size, reserved);
    GCMON_CHECK_NULL(bf1, ERROR);

    bf2 = bf_new(size, reserved);
    GCMON_CHECK_NULL(bf2, ERROR);

    pOuter->bf1 = bf1;
    pOuter->bf2 = bf2;
    pOuter->cur = bf1;

    pOuter->size = size;
    pOuter->reserved = reserved;

    return pOuter;

ERROR:
    bf_free(bf2);
    bf_free(bf1);
    GFREE(pOuter);

    return NULL;
}

GPublic void so_free(SouterP_t pOuter)
{
    GCMON_CHECK_NULL(pOuter, ERROR);

    pOuter->cur = NULL;
    bf_free(pOuter->bf1);
    bf_free(pOuter->bf2);
    GFREE(pOuter);

ERROR:
    return;
}

GPublic void so_switch(SouterP_t pOuter)
{
    GCMON_CHECK_NULL(pOuter, ERROR);

    if (pOuter->cur == pOuter->bf1)
    {
        pOuter->cur = pOuter->bf2;
    }
    else
    {
        GASSERT(pOuter->cur == pOuter->bf2);
        pOuter->cur = pOuter->bf1;
    }

ERROR:
    return;
}

GPublic Int32_t so_print(SouterP_t pOuter, String_t fmt, ...)
{
    Int32_t size = 0;
    va_list args;

    GCMON_CHECK_NULL(pOuter, ERROR);

    if (bf_full(pOuter->cur))
    {
        so_switch(pOuter);
        bf_reset(pOuter->cur);
    }

    va_start(args, fmt);
    size = bf_sprint(pOuter->cur, fmt, args);
    va_end(args);

ERROR:
    return size;
}

GPublic void so_write(SouterP_t pOuter, FILE *file)
{
    GCMON_CHECK_NULL(pOuter, ERROR);
    GCMON_CHECK_NULL(file, ERROR);

    so_switch(pOuter);
    bf_write(pOuter->cur, file);
    so_switch(pOuter);
    bf_write(pOuter->cur, file);

ERROR:
    return;
}

GPrivate struct
{
    String_t szHeader;
    Int32_t sdwType;
    union
    {
        Double_t(*pfnDouble)();
        jlong(*pfnJlong)();
    };
} gaSout[] =
{
    { "Time(sec)", S_FD, s_timstamp },

    { "Generation Space(KB)", S_FS, NULL },

    { "S0C", S_FD, s_s0c },
    { "S1C", S_FD, s_s1c },
    { "EC", S_FD, s_ec },
    { "OC", S_FD, s_oc },
    { "PC", S_FD, s_pc },

    { "Generation Space Usage(KB)", S_FS, NULL },

    { "S0U", S_FD, s_s0u },
    { "S0F", S_FD, s_s0f },
    { "S1U", S_FD, s_s1u },
    { "S1F", S_FD, s_s1f },
    { "EU", S_FD, s_eu },
    { "EF", S_FD, s_ef },
    { "OU", S_FD, s_ou },
    { "OF", S_FD, s_of },
    { "PU", S_FD, s_pu },
    { "PF", S_FD, s_pf },

    { "Generation Space Usage(%)", S_FS, NULL },

    { "S0P", S_FD, s_s0p },
    { "S1P", S_FD, s_s1p },
    { "EP", S_FD, s_ep },
    { "OP", S_FD, s_op },
    { "PP", S_FD, s_pp },

    { "GC Information", S_FS, NULL },

    { "YGC", S_FL, (Double_t(*)())s_ygc },
    { "YGCT", S_FD, s_ygct },
    { "AYGCT", S_FD, s_aygct },
    /*{ "CYGCT", S_FD, s_cygct },*/
    { "YGCTP", S_FD, s_ygctp },

    { "FGC", S_FL, (Double_t(*)())s_fgc },
    { "FGCT", S_FD, s_fgct },
    { "AFGCT", S_FD, s_afgct },
    /*{ "CFGCT", S_FD, s_cfgct },*/
    { "FGCTP", S_FD, s_fgctp },
    { "GCT", S_FD, s_gct },
    { "GCTP", S_FD, s_gctp },

    { "Generation Space(KB)", S_FS, NULL },

    { "NGCMIN", S_FD, s_ngcmn },
    { "NGCMAX", S_FD, s_ngcmx },
    { "NGC", S_FD, s_ngc },
    { "OGCMIN", S_FD, s_ogcmn },
    { "OGCMAX", S_FD, s_ogcmx },
    { "OGC", S_FD, s_ogc },
    { "PGCMIN", S_FD, s_pgcmn },
    { "PGCMAX", S_FD, s_pgcmx },
    { "PGC", S_FD, s_pgc },
};

/*!
*@brief        将gaSout的所有项输出到一行
*@author       zhaohm3
*@param[in]    pFile
*@param[in]    szContext
*@retval
*@note
*
*@since    2014-9-23 17:52
*@attention
*
*/
GPublic void s_out_row(FILE *pFile, String_t szContext)
{
    Int32_t sdwLen = ARRAY_SIZE(gaSout);
    Int32_t i = 0;

    if (NULL == pFile)
    {
        pFile = stdout;
    }

    if (szContext != NULL)
    {
        os_fprintf(pFile, szContext);
    }

    for (i = 0; i < sdwLen; i++)
    {
        switch (gaSout[i].sdwType)
        {
        case S_FD:
            os_fprintf(pFile, "%s: "FMTF"\t", gaSout[i].szHeader, gaSout[i].pfnDouble());
            break;
        case S_FL:
            os_fprintf(pFile, "%s: "FMTL"\t", gaSout[i].szHeader, gaSout[i].pfnJlong());
            break;
        case S_FS:
            os_fprintf(pFile, " | \t");
            break;
        default:
            break;
        }
    }

    os_fprintf(pFile, "\n");
}

GPublic void s_out_row_bf(SouterP_t pSouter, String_t szContext)
{
    Int32_t sdwLen = ARRAY_SIZE(gaSout);
    Int32_t i = 0;

    if (szContext != NULL)
    {
        so_print(pSouter, "%s", szContext);
    }

    for (i = 0; i < sdwLen; i++)
    {
        switch (gaSout[i].sdwType)
        {
        case S_FD:
            so_print(pSouter, "%s: "FMTF"\t", gaSout[i].szHeader, gaSout[i].pfnDouble());
            break;
        case S_FL:
            so_print(pSouter, "%s: "FMTL"\t", gaSout[i].szHeader, gaSout[i].pfnJlong());
            break;
        case S_FS:
            so_print(pSouter, "%s", " | \t");
            break;
        default:
            break;
        }
    }

    so_print(pSouter, "%s", "\n");
}


/*!
*@brief        将gaSout的所有项输出到一列
*@author       zhaohm3
*@param[in]    pFile
*@param[in]    szContext
*@retval
*@note
*
*@since    2014-9-23 17:54
*@attention
*
*/
GPublic void s_out_line(FILE *pFile, String_t szContext)
{
    Int32_t sdwLen = ARRAY_SIZE(gaSout);
    Int32_t i = 0;

    if (NULL == pFile)
    {
        pFile = stdout;
    }

    if (szContext != NULL)
    {
        os_fprintf(pFile, "%s\n", szContext);
    }

    for (i = 0; i < sdwLen; i++)
    {
        switch (gaSout[i].sdwType)
        {
        case S_FD:
            os_fprintf(pFile, "%s: "FMTF"\n", gaSout[i].szHeader, gaSout[i].pfnDouble());
            break;
        case S_FL:
            os_fprintf(pFile, "%s: "FMTL"\n", gaSout[i].szHeader, gaSout[i].pfnJlong());
            break;
        case S_FS:
            os_fprintf(pFile, "\n---> %s <---\n", gaSout[i].szHeader);
            break;
        default:
            break;
        }
    }
}

/*!
*@brief        对外接口，采样输出
*@author       zhaohm3
*@param[in]    szContext
*@retval
*@note
*
*@since    2014-9-22 15:35
*@attention
*
*/
GPublic void sample_doit(String_t szContext)
{
    FILE *pStatFile = file_get_fstat();

    if (stdout == pStatFile)
    {
        s_out_row(pStatFile, szContext);
    }
    else if (pStatFile != NULL)
    {
        s_out_row_bf(gcmon_get_souter(), szContext);
    }
}

GPublic Int32_t JNICALL sample_tdoit(String_t szContext)
{
    while (TRUE)
    {
        sample_doit(szContext);
        os_sleep(1000);
    }
}
