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
/*
 * VerboseStringConsumer.h
 *
 *  Created on: Mar 2, 2011
 *      Author: gtoubassi
 */

#ifndef VERBOSESTRINGCONSUMER_H_
#define VERBOSESTRINGCONSUMER_H_

#include <string>

#include "SubstringPacker.h"

using namespace std;

namespace femtozip {

class VerboseStringConsumer : public SubstringPacker::Consumer {
private:
    string output;

public:
    VerboseStringConsumer();
    ~VerboseStringConsumer();

    void encodeLiteral(int aByte, void *context);
    void encodeSubstring(int offset, int length, void *context);
    void endEncoding(void *context);

    string getOutput();
};

}

#endif /* VERBOSESTRINGCONSUMER_H_ */
