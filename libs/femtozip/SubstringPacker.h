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
 * SubstringPacker.h
 *
 *  Created on: Mar 2, 2011
 *      Author: gtoubassi
 */
/*
 *   This file was modified in March 2019 by Sander in 't Veld
 *   (sander@abunchofhacks.coop): GCC pragmas were added to avoid
 *   certain compilation warnings.
 */

#ifndef SUBSTRINGPACKER_H_
#define SUBSTRINGPACKER_H_

#include <string>
#include "PrefixHash.h"

using namespace std;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

namespace femtozip {

class SubstringPacker {

private:
    int compressionLevel;
    const char *dict;
    int dictLen;
    PrefixHash *dictHash;

public:

    class Consumer {
    public:
        virtual void encodeLiteral(int aByte, void *context) = 0;
        virtual void encodeSubstring(int offset, int length, void *context) = 0;
        virtual void endEncoding(void *context) = 0;
    };

    SubstringPacker(const char *dictionary, int length, int compressionLevel = 9 /* 0 faster ... 9 better */);
    ~SubstringPacker();

    void pack(const char *bytes, int length, Consumer& consumer, void *consumerContext);
};


}

#pragma GCC diagnostic pop

#endif /* SUBSTRINGPACKER_H_ */
