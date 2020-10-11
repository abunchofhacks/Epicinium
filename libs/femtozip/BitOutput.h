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
 * BitOutput.h
 *
 *  Created on: Mar 2, 2011
 *      Author: gtoubassi
 */

#ifndef BITOUTPUT_H_
#define BITOUTPUT_H_

#include <vector>

using namespace std;

namespace femtozip {

class BitOutput {

private:
    vector<char>& out;
    char buffer;
    int count;


public:
    BitOutput(vector<char>& output) : out(output), buffer(0), count(0) {};

    void writeBit(int bit) {
        if (bit) {
            buffer |= (1 << count);
        }
        count++;
        if (count == 8) {
            out.push_back(buffer);
            buffer = 0;
            count = 0;
        }
    }

    void flush() {
        if (count > 0) {
            out.push_back(buffer);
            buffer = 0;
            count = 0;
        }
    }
};

}

#endif /* BITOUTPUT_H_ */
