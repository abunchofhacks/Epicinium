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

#ifndef OFFSETNIBBLEHUFFMANCOMPRESSIONMODEL_H_
#define OFFSETNIBBLEHUFFMANCOMPRESSIONMODEL_H_

#include <iostream>

#include "CompressionModel.h"
#include "DocumentList.h"
#include "HuffmanEncoder.h"
#include "FrequencyHuffmanModel.h"
#include "SubstringPacker.h"

namespace femtozip {

class FemtoZipCompressionModel : public CompressionModel {
private:
    FrequencyHuffmanModel *literalLengthModel;
    FrequencyHuffmanModel *offsetNibble0Model;
    FrequencyHuffmanModel *offsetNibble1Model;
    FrequencyHuffmanModel *offsetNibble2Model;
    FrequencyHuffmanModel *offsetNibble3Model;

protected:
    virtual SubstringPacker::Consumer *createModelBuilder();

public:
    FemtoZipCompressionModel();
    virtual ~FemtoZipCompressionModel();

    virtual void load(DataInput& in);
    virtual void save(DataOutput& out);

    virtual const char *typeName() { return "FemtoZip"; };

    virtual void build(DocumentList& documents);

    virtual void compress(const char *buf, int length, ostream& out);
    virtual void decompress(const char *buf, int length, ostream& out);

    virtual void encodeLiteral(int aByte, void *context);
    virtual void encodeSubstring(int offset, int length, void *context);
    virtual void endEncoding(void *context);

    friend class FemtoZipHuffmanModel;
};

}

#endif /* OFFSETNIBBLEHUFFMANCOMPRESSIONMODEL_H_ */
