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

#ifndef PMPMEAS_MEAS_H
#define PMPMEAS_MEAS_H

#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <limits>
#include <vector>
#include <string>
#include "meastypes.hpp"
#include "papiinf.hpp"
#include "perfinf.hpp"
#include "logger.hpp"

namespace PMPMEAS {

    class Meas
    {
        private:
            std::string _tag;               //!< User defined tag
            MeasType _type;                 //!< Type of counters
            static PapiInf _papi;           //!< Reference to PAPI context
            static PerfInf _perf;           //!< Reference to perf context
            int _cnt;                       //!< Number of counters
            float _cweight;                 //!< Current weight
            float _avweight;                //!< Average weight
            std::vector<long long> _val;    //!< Current counter values
            std::vector<long long> _t[2];   //!< Counter values (0=start, 1=end)
            std::vector<double> _mean;      //!< Average counter values
            std::vector<double> _min;       //!< Minimum counter values
            std::vector<double> _max;       //!< Maximum counter values
            int _nmeas;                     //!< Number of measurements

        public:
            Meas(const char* tag, MeasType type)
                : _tag(tag), _type(type), _cweight(0), _avweight(0), _nmeas(0)
            {
                if (type() == MeasType::PAPI)
                {
                    if (_papi.nevent() == 0)
                    {
                        for (int i = 0; i < type.cnt(); i++)
                        {
                            _papi.create(_type.typestr(i));
                            _cnt++;
                        }
                    }
                    _cnt = _papi.nevent();
                }
                else if (type() == MeasType::PERF)
                {
                    if (_perf.nevent() == 0)
                    {
                        for (int i = 0; i < type.cnt(); i++)
                        {
                            _perf.create(_type.typestr(i));
                            _cnt++;
                        }
                    }
                    _cnt = _perf.nevent();
                }
                else
                {
                    _cnt = 1;
                }

                _val.resize(_cnt);
                _t[0].resize(_cnt);
                _t[1].resize(_cnt);
                _mean.resize(_cnt);
                _min.resize(_cnt);
                _max.resize(_cnt);

                for (int i = 0; i < _cnt; i++)
                {
                    _val[i]  = 0;
                    _t[0][i] = 0;
                    _t[1][i] = 0;
                    _min[i]  = std::numeric_limits<double>::max();
                    _max[i]  = 0;
                }
            }

            inline const string tag(void) const
            {
                return _tag;
            }
    
            inline int cnt(void) const
            {
                return _cnt;
            }

            inline void start()
            {
                _get(0);
            }

            inline void stop(float weight = 1.0)
            {
                _cweight = weight;
                _get(1);
            }

            inline void read()
            {
                _read();
            }
    
            inline long long val(int i) const
            {
                return (i < _cnt) ? _val[i] : 0;
            }

            void dump(FILE *fp);

            double mean(int i=0) const
            {
                return _mean[i] / _nmeas;
            }

            double min(int i=0) const
            {
                return _min[i];
            }

            double max(int i=0) const
            {
                return _max[i];
            }

            int nmeas() const
            {
                return _nmeas;
            }

            float avweight() const
            {
                return _avweight / _nmeas;
            }

            const char* ename(int i)
            {
                const char* ename;
                if (i >= _cnt)
                { 
                    logger.qdie("Invalid index for ename");
                }

                switch( _type() ) 
                {
                    case (MeasType::PAPI):
                        ename = _papi.ename(i);    
                        break;
                    case (MeasType::PERF):
                        ename = _perf.ename(i);    
                        break;
                    case (MeasType::TIME_BOOT):
                        ename = "TIME_BOOT";
                        break;
                    case (MeasType::TIME_CPU):
                        ename = "TIME_CPU";
                        break;
                    case (MeasType::TIME_THRD):
                        ename = "TIME_CPU";
                        break;
                    default: // unrecognized
                        logger.qdie("Unrecognized Meas type");
                }
                return(ename);
            }


        private:
            long long _time_boot(void)
            {
                struct timespec ts;
                clock_gettime(CLOCK_BOOTTIME, &ts);
                return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
            }

            long long _time_cpu(void)
            {
                struct timespec ts;
                clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
                return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
            }

            long long _time_thrd(void)
            {
                struct timespec ts;
                clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
                return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
            }

            void _get(int i)
            {
                assert (i == 0 || i == 1);

                switch(_type())
                {
                    case MeasType::TIME_BOOT:
                    {
                        _t[i][0] = _time_boot();
                        break;
                    }

                    case MeasType::TIME_CPU:
                    {
                        _t[i][0] = _time_cpu();
                        break;
                    }

                    case MeasType::TIME_THRD:
                    {
                        _t[i][0] = _time_thrd();
                        break;
                    }

                    case MeasType::PAPI:
                    {
                        if (i == 0)
                        {
                            _papi.start();
                            for (int j = 0; j < _cnt; j++)
                                _t[0][j] = 0;
                        }
                        else
                        {
                            _papi.stop();
                            for (int j = 0; j < _cnt; j++)
                                _t[1][j] = _papi.eval(j);
                        }
                        break;
                    }

                    case MeasType::PERF:
                    {
                        if (i == 0)
                        {
                            _perf.start();
                            for (int j = 0; j < _cnt; j++)
                                _t[0][j] = 0;
                        }
                        else
                        {
                            _perf.stop();
                            for (int j = 0; j < _cnt; j++)
                            {
                                _t[1][j] = _perf.eval(j);
                            }
                        }
                        break;
                    }

                    default:
                        logger.qdie("Internal error");
                }

                if (i == 1)
                {
                    for (int j = 0; j < _cnt; j++)
                    {
                        double d = (_t[1][j] - _t[0][j]) * _cweight;
                        _mean[j] += d;
                        if (d < _min[j])
                            _min[j] = d;
                        if (d > _max[j])
                            _max[j] = d;
                    }

                    _avweight += _cweight;
                    _nmeas++;
                }
            }

	        void _read(void)
	        {
                if (_cnt < 1)
                    return;

                switch (_type())
                {
                    case MeasType::TIME_BOOT:
                    {
                        _val[0] = _time_boot();
                        break;
                    }

                    case MeasType::TIME_CPU:
                    {
                        _val[0] = _time_cpu();
                        break;
                    }

                    case MeasType::TIME_THRD:
                    {
                        _val[0] = _time_thrd();
                        break;
                    }

                    case MeasType::PAPI:
                    {
			            _papi.read();
                        for (int i = 0; i < _cnt; ++i)
                            _val[i] = _papi.eval(i);
                        break;
                    }

                    case MeasType::PERF:
                    {
			            _perf.read();
                        for (int i = 0; i < _cnt; ++i)
                            _val[i] = _perf.eval(i);
                        break;
                    }

                    default:
                        logger.qdie("Internal error");
                }
	        }
    };

}

#endif
