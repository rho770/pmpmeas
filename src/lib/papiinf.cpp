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

#include "papiinf.hpp"
#include "logger.hpp"

using namespace PMPMEAS;

int PapiInf::_cnt = 0;

PapiInf::PapiInf(void)
{
#if PMPMEAS_USEPAPI
    _cnt++;
    if (_cnt > 1)
        logger.qdie("At most one object of class PapiInf must be instantiated!");

    if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT)
        logger.qdie("PAPI library init error!");

    _ctx = PAPI_NULL;
    auto ret = PAPI_create_eventset(&_ctx);
    if (ret != PAPI_OK)
        logger.qvdie("Failed to create event set (%s)", PAPI_strerror(ret));
#endif

    _nevent = 0;
}

int PapiInf::create(const std::string& ename)
{
#if PMPMEAS_USEPAPI
    if (_nevent == PAPICNTMAX)
        logger.qdie("Exceeding upper limit of number of PAPI events");

    auto ret = PAPI_add_named_event(_ctx, (char *)ename.c_str());
    if (ret != PAPI_OK)
    {
        logger.qverror("PAPI_add_named_event failed (%s,\"%s\")", PAPI_strerror(ret) % ename.c_str());
        return -1;
    }

    _ename[_nevent] = ename.c_str();
    _nevent++;
     logger.qvdebug("ename=%s, nevent=%d", ename.c_str() % _nevent);

    return (_nevent - 1);
#else
    return 0;
#endif
}

void PapiInf::cleanup()
{
#if PMPMEAS_USEPAPI
    auto ret = PAPI_cleanup_eventset(_ctx);
    if (ret != PAPI_OK)
        logger.qvdie("PAPI_cleanup_eventset failed (%s)\n", PAPI_strerror(ret));
#endif

    _nevent = 0;
}

void PapiInf::start(void)
{
#if PMPMEAS_USEPAPI
    auto ret = PAPI_start(_ctx);
    if (ret != PAPI_OK)
        logger.qdie("PAPI_start failed\n");
#endif
}

void PapiInf::stop(void)
{
#if PMPMEAS_USEPAPI
    auto ret = PAPI_stop(_ctx, _eval);
    if (ret != PAPI_OK)
        logger.qvdie("PAPI_stop failed (%s)", ret);
#endif
}

void PapiInf::read(void)
{
#if PMPMEAS_USEPAPI
    auto ret = PAPI_read(_ctx, _eval);
    if (ret != PAPI_OK)
        logger.qvdie("PAPI_stop failed (%s)", ret);
#endif
}