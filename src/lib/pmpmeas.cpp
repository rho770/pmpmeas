/* -------
 * PMPMEAS
 * -------
 *
 * Copyright 2022 Dirk Pleiter (dirk.pleiter@protonmail.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. The origin of this software must not be misrepresented; you must
 *    not claim that you wrote the original software.  If you use this
 *    software in a product, an acknowledgment in the product
 *    documentation would be appreciated but is not required.
 *
 * 3. Altered source versions must be plainly marked as such, and must
 *    not be misrepresented as being the original software.
 *
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <list>
#include "pmpmeas.hpp"
#include "meas.hpp"
#include "logger.hpp"

using namespace std;
using namespace PMPMEAS;

list<MeasType*> pmpmeas_type_lst;
list<Meas*> pmpmeas_meas_lst;
list<Meas*> pmpmeas_match_lst;

string ctag;

time_t tstart;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Library initialisation function
 */

void pmpmeas__init()
{
    tstart = time(NULL);

    char* t = getenv("PMPMEAS_MEAS_TYPES");
    if (t != NULL)
    {
        char *c1 = t;
        do {
            char *c0 = c1;
            for (; (*c1 != '\0') && (*c1 != ';'); c1++);
            if (*c1 == ';')
            {
                *c1 = '\0';
                pmpmeas_type_lst.push_back(new MeasType(c0));
            }
            else
            {
                pmpmeas_type_lst.push_back(new MeasType(c0));
                break;
            }
        } while (c1++);
    }
}

/*
 * Start measurement
 */

void pmpmeas__start(const char *tag)
{
    ctag = tag;

    pmpmeas_match_lst.clear();

    for (list<Meas*>::iterator m = pmpmeas_meas_lst.begin(); m != pmpmeas_meas_lst.end(); m++)
        if ((*m)->tag() == ctag)
            pmpmeas_match_lst.push_back(*m);

    if (pmpmeas_match_lst.empty())
        for (list<MeasType*>::iterator mt = pmpmeas_type_lst.begin(); mt != pmpmeas_type_lst.end(); mt++)
        {
            Meas *m = new Meas(tag, *(*mt));
            pmpmeas_meas_lst.push_back(m);
            pmpmeas_match_lst.push_back(m);
        }

    for (list<Meas*>::iterator m = pmpmeas_match_lst.begin(); m != pmpmeas_match_lst.end(); m++)
        (*m)->start();
}

void pmpmeas__stop(float weight)
{
    for (list<Meas*>::iterator m = pmpmeas_match_lst.begin(); m != pmpmeas_match_lst.end(); m++)
        (*m)->stop(weight);
}

void pmpmeas__finish()
{
    const int len = 1024;
    struct tm * timeinfo;
    char buf[len];
    char fname[len];
    pid_t pid = getpid();

    snprintf(fname, len, "pmpmeas_%06d_XXXXXX", pid);
    mkstemp(fname);
    FILE *fp = fopen(fname, "w");
    if (fp == NULL)
        logger.qvdie("Failed to open file \"%s\" for writing\n", fname);

    timeinfo = localtime(&tstart);
    strftime (buf, len, "%F %T", timeinfo);
    fprintf(fp, "Start:       %s\n", buf);
    fprintf(fp, "\n");

    gethostname(buf, len);
    fprintf(fp, "Hostname:    %s\n", buf);
    fprintf(fp, "\n");

    for (list<Meas*>::iterator m = pmpmeas_meas_lst.begin(); m != pmpmeas_meas_lst.end(); m++)
        (*m)->dump(fp);

    time_t tend = time(NULL);
    timeinfo = localtime(&tend);
    strftime (buf, len, "%F %T", timeinfo);
    fprintf(fp, "End:         %s\n", buf);

    fclose(fp);
}


void pmpmeas__valcnt(int *cnt)
{
    *cnt = 0;

    for (list<Meas*>::iterator m = pmpmeas_meas_lst.begin(); m != pmpmeas_meas_lst.end(); m++)
        *cnt += (*m)->cnt();
}

void pmpmeas__valrd(pmpmeas_vlst_t *vlst)
{
    vlst->cnt = 0;
    for (list<Meas *>::iterator m = pmpmeas_meas_lst.begin(); m != pmpmeas_meas_lst.end(); m++)
    {
        (*m)->read();
        for (auto i = 0; i < (*m)->cnt(); i++)
            if (vlst->cnt < vlst->n)
            {
                vlst->val[vlst->cnt] += (*m)->val(i);
                vlst->cnt++;
            }
    }
}

#ifdef __cplusplus
}
#endif
