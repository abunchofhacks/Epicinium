/**
 *   Copyright 2011 Garrick Toubassi
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <sys/time.h>

namespace femtozip {

class Util {
public:
    static long long getMillis() {
        timeval tim;
        gettimeofday(&tim, NULL);
        return tim.tv_sec * 1000LL + tim.tv_usec / 1000LL;
    }
};

}

#endif /* UTIL_H_ */
