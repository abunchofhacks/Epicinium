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
 * CompressionModel.h
 *
 *  Created on: Mar 4, 2011
 *      Author: gtoubassi
 */
/*
 *   This file was modified in March 2019 by Sander in 't Veld
 *   (sander@abunchofhacks.coop): GCC pragmas were added to avoid
 *   certain compilation warnings.
 */

#ifndef COMPRESSIONMODEL_H_
#define COMPRESSIONMODEL_H_

#include <iostream>
#include "SubstringPacker.h"
#include "DocumentList.h"
#include "DataIO.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

namespace femtozip {

class CompressionModel : public SubstringPacker::Consumer {
protected:
    int compressionLevel;
    const char *dict;
    int dictLen;
    SubstringPacker *packer;

    virtual void buildDictionaryIfUnspecified(DocumentList& documents);
    SubstringPacker *getSubstringPacker();

    virtual SubstringPacker::Consumer *createModelBuilder();

    virtual SubstringPacker::Consumer *buildEncodingModel(DocumentList& documents);

public:

    static CompressionModel *buildOptimalModel(DocumentList& documents, bool verify = false, vector<string> *modelTypes = 0);
    static CompressionModel *createModel(const string& type);
    static void saveModel(CompressionModel& model, DataOutput& out);
    static CompressionModel *loadModel(DataInput& in);

    CompressionModel(int compressionLevel = 9);
    virtual ~CompressionModel();

    /**
     * compressionLevel is in range [0..9] where 0 means
     * faster, and 9 means better/smaller.
     */
    void setCompressionLevel(int level);
    int getCompressionLevel();

    void setMaxDictionary(int maxDictionary);

    virtual void load(DataInput& in);
    virtual void save(DataOutput& out);

    virtual const char *typeName() = 0;

    virtual void setDictionary(const char *dictionary, int length);
    virtual const char *getDictionary(int& length);

    virtual void build(DocumentList& documents) = 0;

    virtual void compress(const char *buf, int length, ostream& out);
    virtual void decompress(const char *buf, int length, ostream& out) = 0;

    virtual void encodeLiteral(int aByte, void *context) = 0;
    virtual void encodeSubstring(int offset, int length, void *context) = 0;
    virtual void endEncoding(void *context) = 0;
};

}

#pragma GCC diagnostic pop

#endif /* COMPRESSIONMODEL_H_ */
