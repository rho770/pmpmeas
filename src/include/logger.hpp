/* -------
 * PMPMEAS
 * -------
 *
 * Copyright 2024 Dirk Pleiter <dirk.pleiter@protonmail.com>
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

#ifndef PMPMEAS_LOGGER_HH
#define PMPMEAS_LOGGER_HH

#include <stdio.h>
#include <stdlib.h>
#include <boost/format.hpp>

#define die(fmt)               PMPMEAS::_die(__FILE__, __LINE__, boost::format(fmt))
#define vdie(fmt, ARGS...)     PMPMEAS::_die(__FILE__, __LINE__, boost::format(fmt) % ARGS)

#ifdef PMPMEAS_USERTRACE
void report_and_exit(const char*);
#endif

namespace PMPMEAS {

    //! Class for logging
    class Logger {
        public:
            enum Type
            {
#define         xx(a,b,c) a = b,
#include        "loggerlevelxx.h"
            };

        private:
            int         _level;
            std::string _category;
            static bool _first;

        public:
            Logger(int level = Logger::error)
            : _level(level)
            {};
            
            void setlevel(Type level)
            {
                _level = level;
            }

#define     xx(a,b,c) void _##a(const char*, int, const boost::format&);
#include    "loggerlevelxx.h"
    };

    extern Logger logger;
}

#define qdie(fmt)               _die(__FILE__, __LINE__, boost::format(fmt))
#define qvdie(fmt, ARGS...)     _die(__FILE__, __LINE__, boost::format(fmt) % ARGS)
#define qcrit(fmt)              _crit(__FILE__, __LINE__, boost::format(fmt))
#define qvcrit(fmt, ARGS...)    _crit(__FILE__, __LINE__, boost::format(fmt) % ARGS)
#define qfatal(fmt)             _fatal(__FILE__, __LINE__, boost::format(fmt))
#define qvfatal(fmt, ARGS...)   _fatal(__FILE__, __LINE__, boost::format(fmt) % ARGS)
#define qerror(fmt)             _error(__FILE__, __LINE__, boost::format(fmt))
#define qverror(fmt, ARGS...)   _error(__FILE__, __LINE__, boost::format(fmt) % ARGS)
#define qnotice(fmt)            _notice(__FILE__, __LINE__, boost::format(fmt))
#define qvnotice(fmt, ARGS...)  _notice(__FILE__, __LINE__, boost::format(fmt) % ARGS)
#define qinfo(fmt)              _info(__FILE__, __LINE__, boost::format(fmt))
#define qvinfo(fmt, ARGS...)    _info(__FILE__, __LINE__, boost::format(fmt) % ARGS)
#define qdebug(fmt)             _debug(__FILE__, __LINE__, boost::format(fmt))
#define qvdebug(fmt, ARGS...)   _debug(__FILE__, __LINE__, boost::format(fmt) % ARGS)

#endif