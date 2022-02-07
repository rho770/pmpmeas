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

/*
 * Code for access to perf counters
*/

#include "perfinf.hh"
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

int PerfInf::_cnt = 0;


struct read_format {
    uint64_t nr;
    struct {
        uint64_t value;
        uint64_t id;
    } values[];
};


PerfInf::PerfInf(void)
{
    int ret;

    _cnt++;
    if (_cnt > 1)
    {
        fprintf(stderr, "At most one object of class PerfInf must be instantiated!\n");
        exit(1);
    }

    _nevent = 0;
}


int PerfInf::create(const std::string& ename)
{
    if (_nevent >= PERF_CNTMAX) {
        fprintf(stderr, "Maximum number of PerfInf events exceeded\n");
        exit(1);
    }

    struct perf_event_attr pea;
    memset(&pea, 0, sizeof(struct perf_event_attr));

    pea.size = sizeof(struct perf_event_attr);

    if (0) {}
#   define xx(a, b, c) else if (ename == #a) { pea.type = b; pea.config = c; }
#   include "perfinftypesxx.h"
    else
    {
        fprintf(stderr, "Unknown perf counter type \"%s\"\n", ename.c_str());
        exit(1);
    }

    pea.disabled = 1;
    pea.exclude_kernel = 1;
    pea.exclude_hv = 1;
    pea.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID;

    if (_nevent == 0) {
        _fd[0]       = syscall(__NR_perf_event_open, &pea, 0, -1, -1,     0);
    }
    else {
        _fd[_nevent] = syscall(__NR_perf_event_open, &pea, 0, -1, _fd[0], 0);
    }
    if (_fd[_nevent] == -1) {
        fprintf(stderr, "WARNING: PerfInf failed to create event (ename=%s, config=0x%llx)\n", ename.c_str(), pea.config);
        return -1;
    }

    if (ioctl(_fd[_nevent], PERF_EVENT_IOC_ID, &_eid[_nevent]) != 0)
    {
        fprintf(stderr, "ioctl failed\n");
        exit(1);
    }

    _nevent++;

    return (_nevent - 1);
}

void PerfInf::cleanup(void)
{
    int i;

    for (i = 0; i < _nevent; i++)
        close(_fd[_nevent]);
}


void PerfInf::start(void)
{
    if (_nevent > 0) {
        ioctl(_fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
        ioctl(_fd[0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
    }
}


void PerfInf::stop(void)
{
    if (_nevent > 0) {
        if (ioctl(_fd[0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP) != 0)
        {
            fprintf(stderr, "ioctl failed\n");
            exit(1);
        }
        if (read(_fd[0], _buf, sizeof(_buf)) == -1)
        {
            fprintf(stderr, "read failed\n");
            exit(1);
        }

        for (int j = 0; j < _nevent; j++)
            _eval[j] = 0;

        struct read_format* rf = (struct read_format*) _buf;
        for (int i = 0; i < rf->nr; i++)
            for (int j = 0; j < _nevent; j++)
                if (rf->values[i].id == _eid[j]) {
                    _eval[j] = rf->values[i].value;
                }
    }
}