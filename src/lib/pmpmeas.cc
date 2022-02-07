/* -------
 * PMPMEAS
 * -------
 *
 * Copyright 2022 Dirk Pleiter (pleiter@kth.se)
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
#include "pmpmeas.hh"
#include "meas.hh"

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
    char* t = getenv("PMPMEAS_MEAS_TYPES");
    if (t != NULL) {
        char *c1 = t;
        do {
            char *c0 = c1;
            for (; (*c1 != '\0') && (*c1 != ';'); c1++);
            if (*c1 == ';') {
                *c1 = '\0';
//fprintf(stderr, "DEBUG: Measuring \"%s\"\n", c0);
                pmpmeas_type_lst.push_back(new MeasType(c0));
            }
            else {
//fprintf(stderr, "DEBUG: Measuring \"%s\"\n", c0);
                pmpmeas_type_lst.push_back(new MeasType(c0));
                break;
            }
        } while (c1++);
    }
}

/*
 * Start measurement
 */

void pmpmeas__start(const char *tag, float weight)
{
    ctag = tag;
    tstart = time(NULL);

//fprintf(stderr, "DEBUG: pmpmeas_start() tag=[%s]\n", ctag.c_str());

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
        (*m)->start(weight);
}

void pmpmeas__stop()
{
//fprintf(stderr, "DEBUG: pmpmeas_stop() tag=[%s]\n", ctag.c_str());

    for (list<Meas*>::iterator m = pmpmeas_match_lst.begin(); m != pmpmeas_match_lst.end(); m++)
        (*m)->stop();
}

void pmpmeas__finish()
{
//fprintf(stderr, "DEBUG: pmpmeas_finish()\n");
    struct tm * timeinfo;
    char buf[1024];
    char fname[1024];
    pid_t pid = getpid();

    snprintf(fname, 1024, "pmpmeas_%06d_XXXXXX", pid);
    mkstemp(fname);
//fprintf(stderr, "DEBUG[%s,%d] fname=%s\n", __FILE__, __LINE__, fname);
    FILE *fp = fopen(fname, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file \"%s\" for writing\n", fname);
        exit(1);
    }

    timeinfo = localtime(&tstart);
    strftime (buf, 1024, "%F %T", timeinfo);
    fprintf(fp, "Start:       %s\n\n", buf);

    for (list<Meas*>::iterator m = pmpmeas_meas_lst.begin(); m != pmpmeas_meas_lst.end(); m++)
        (*m)->dump(fp);

    time_t tend = time(NULL);
    timeinfo = localtime(&tend);
    strftime (buf, 1024, "%F %T", timeinfo);
    fprintf(fp, "End:         %s\n", buf);

    fclose(fp);
}

#ifdef __cplusplus
}
#endif