/*
thot package for statistical machine translation
Copyright (C) 2017 Daniel Ortiz-Mart\'inez, Adam Harasimowicz

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file IncrLexLevelDbTable.cc
 * 
 * @brief Definitions file for IncrLexLevelDbTable.h
 */

//--------------- Include files --------------------------------------

#include "IncrLexLevelDbTable.h"

//--------------- IncrLexLevelDbTable class function definitions

//-------------------------
IncrLexLevelDbTable::IncrLexLevelDbTable(void)
{
    // Define extensions
    ldbExtension = "_ldb_hmm_lexnd";
    defaultExtension = ".hmm_lexnd";

    // Prepare DB configuration
    options.create_if_missing = true;
    options.max_open_files = 20;
    options.filter_policy = leveldb::NewBloomFilterPolicy(1);
    options.block_cache = leveldb::NewLRUCache(0.5 * 1048576);  // 0.5 MB for cache
    db = NULL;
    dbName = "";
}

//-------------------------
bool IncrLexLevelDbTable::init(const char* prefFileName)
{
    dbName = ((std::string) prefFileName) + ldbExtension;
    std::cerr << "Initializing LevelDB phrase table in " << dbName << std::endl;

    // Release resources related to old DB if exists
    if(db != NULL)
    {
        delete db;
        db = NULL;
    }

    // Prepare empty DB
    leveldb::Status status = leveldb::DB::Open(options, dbName, &db);

    if (status.ok())
    {
        clear();

        return THOT_OK;
    }
    else
    {
        std::cerr << "Cannot open DB: " << status.ToString() << std::endl;

        return THOT_ERROR;
    }
}

//-------------------------
bool IncrLexLevelDbTable::drop()
{
    if(db != NULL)
    {
        delete db;
        db = NULL;
    }

    leveldb::Status status = leveldb::DestroyDB(dbName, options);

    if(status.ok())
    {
        return THOT_OK;
    }
    else
    {
        std::cerr << "Dropping database status: " << status.ToString() << std::endl;

        return THOT_ERROR;
    }
}

//-------------------------
std::string IncrLexLevelDbTable::vectorToString(const std::vector<WordIndex>& vec)const
{
    std::vector<WordIndex> str;
    for(size_t i = 0; i < vec.size(); i++) {
        // Use WORD_INDEX_MODULO_BYTES bytes to encode index
        for(int j = WORD_INDEX_MODULO_BYTES - 1; j >= 0; j--) {
            str.push_back(1 + (vec[i] / (unsigned int) pow(WORD_INDEX_MODULO_BASE, j) % WORD_INDEX_MODULO_BASE));
        }
    }

    std::string s(str.begin(), str.end());

    return s;
}

//-------------------------
std::vector<WordIndex> IncrLexLevelDbTable::stringToVector(const std::string s)const
{
    std::vector<WordIndex> vec;

    for(size_t i = 0; i < s.size();)  // A string length is WORD_INDEX_MODULO_BYTES * n + 1
    {
        unsigned int wi = 0;
        for(int j = WORD_INDEX_MODULO_BYTES - 1; j >= 0; j--, i++) {
            wi += (((unsigned char) s[i]) - 1) * (unsigned int) pow(WORD_INDEX_MODULO_BASE, j);
        }

        vec.push_back(wi);
    }

    return vec;
}

//-------------------------
std::string IncrLexLevelDbTable::vectorToKey(const std::vector<WordIndex>& vec)const
{
    return vectorToString(vec);
}

//-------------------------
std::vector<WordIndex> IncrLexLevelDbTable::keyToVector(const std::string key)const
{
    return stringToVector(key);
}

//-------------------------
bool IncrLexLevelDbTable::stringToFloat(const std::string value_str, float &value)const
{
    // Decode string representation to float without loosing precision
    unsigned int wi = 0;
    for(size_t i = 0; i < WORD_INDEX_MODULO_BYTES; i++) {
        int j = WORD_INDEX_MODULO_BYTES - i - 1;
        wi += (((unsigned char) value_str[i]) - 1) * (unsigned int) pow(WORD_INDEX_MODULO_BASE, j);
    }

    float *p = reinterpret_cast<float*>(&wi);
    value = *p;

    return true;;
}

//-------------------------
std::string IncrLexLevelDbTable::floatToString(const float value)const
{
    // Encode float as a string without loosing precision
    unsigned int const *p = reinterpret_cast<unsigned int const*>(&value);

    std::vector<WordIndex> str;
    for(int j = WORD_INDEX_MODULO_BYTES - 1; j >= 0; j--) {
        str.push_back(1 + (*p / (unsigned int) pow(WORD_INDEX_MODULO_BASE, j) % WORD_INDEX_MODULO_BASE));
    }

    std::string s(str.begin(), str.end());

    return s;
}

//-------------------------
bool IncrLexLevelDbTable::retrieveData(const std::vector<WordIndex>& phrase, float &value)const
{
    std::string value_str;
    value = 0;
    std::string key = vectorToString(phrase);

    leveldb::Status result = db->Get(leveldb::ReadOptions(), key, &value_str);

    if (result.ok()) {
        return stringToFloat(value_str, value);
    } else {
        return false;
    }
}

//-------------------------
bool IncrLexLevelDbTable::storeData(const std::vector<WordIndex>& phrase, float value)const
{
    std::string value_str = floatToString(value);

    leveldb::WriteBatch batch;
    batch.Put(vectorToString(phrase), value_str);
    leveldb::Status s = db->Write(leveldb::WriteOptions(), &batch);

    if(!s.ok())
        std::cerr << "Storing data status: " << s.ToString() << std::endl;

    return s.ok();
}

//-------------------------
void IncrLexLevelDbTable::setLexNumer(WordIndex s,
                                      WordIndex t,
                                      float f)
{
    // Insert lexNumer for pair (s,t)
    // Due to performance of getTransForTarget method,
    // pair (s,t) is stored as (t,s) in DB
    std::vector<WordIndex> st_vec;
    st_vec.push_back(t);
    st_vec.push_back(s);

    storeData(st_vec, f);
}

//-------------------------
float IncrLexLevelDbTable::getLexNumer(WordIndex s,
                                       WordIndex t,
                                       bool& found)
{
    float lexNumber;
    std::vector<WordIndex> st_vec;
    st_vec.push_back(t);
    st_vec.push_back(s);

    found = retrieveData(st_vec, lexNumber);

    return lexNumber;
}

//-------------------------
void IncrLexLevelDbTable::setLexDenom(WordIndex s,
                                      float d)
{
    std::vector<WordIndex> s_vec;
    s_vec.push_back(s);

    storeData(s_vec, d);
}

//-------------------------
float IncrLexLevelDbTable::getLexDenom(WordIndex s,
                                       bool& found)
{
    float lexDenom;
    std::vector<WordIndex> s_vec;
    s_vec.push_back(s);

    found = retrieveData(s_vec, lexDenom);

    return lexDenom;
}

//-------------------------
bool IncrLexLevelDbTable::getTransForTarget(WordIndex t,
                                            std::set<WordIndex>& transSet)
{
    bool found;

    std::vector<WordIndex> start_vec;
    start_vec.push_back(t);
    std::vector<WordIndex> end_vec;
    end_vec.push_back(t + 1);

    std::string start_str = vectorToKey(start_vec);
    std::string end_str = vectorToKey(end_vec);

    leveldb::Slice start = start_str;
    leveldb::Slice end = end_str;

    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());

    transSet.clear();

    for(it->Seek(start); it->Valid() && it->key().ToString() < end.ToString(); it->Next()) {
        std::vector<WordIndex> vec = keyToVector(it->key().ToString());

        if(vec.size() > 1) {  // Skip single word entry, e.g. (t)
            WordIndex s = vec[vec.size() - 1];
            transSet.insert(s);
        }
    }

    found = it->status().ok();

    delete it;

    return transSet.size() > 0 && found;
}

//-------------------------
void IncrLexLevelDbTable::setLexNumDen(WordIndex s,
                                       WordIndex t,
                                       float num,
                                       float den)
{
    setLexDenom(s, den);
    setLexNumer(s, t, num);
}

//-------------------------
bool IncrLexLevelDbTable::load(const char* lexNumDenFile)
{
    bool ret;

    // Try to load LevelDB
    ret = loadLevelDb(lexNumDenFile);

    if (ret == THOT_ERROR)  // Loading DB failed. Look for binary file
    {
        ret = loadBin(lexNumDenFile);
    }

    return ret;
}

//-------------------------
bool IncrLexLevelDbTable::loadBin(const char* lexNumDenFile)
{
    const std::string prefixFile = ((std::string) lexNumDenFile);
    const std::string binFile = prefixFile + defaultExtension;

    if(db != NULL)
    {
        delete db;
        db = NULL;
    }

    // Prepare new DB
    init(prefixFile.c_str());

    std::cerr << "Loading lexnd in LevelDB format from binary file in " << binFile << std::endl;

    std::ifstream inF (binFile.c_str(), std::ios::in | std::ios::binary);
    if (!inF)
    {
        std::cerr << "Error in lexical nd file, file " << binFile << " does not exist. ";

        return THOT_ERROR;
    }

    // Read data stored in binary file and insert them to LevelDB
    bool end = false;
    while(!end)
    {
        WordIndex s;
        WordIndex t;
        float numer;
        float denom;

        if(inF.read((char*) &s, sizeof(WordIndex)))
        {
            inF.read((char*) &t, sizeof(WordIndex));
            inF.read((char*) &numer, sizeof(float));
            inF.read((char*) &denom, sizeof(float));

            setLexNumDen(s, t, numer, denom);
        }
        else end = true;
    }

    return THOT_OK;
}

//-------------------------
bool IncrLexLevelDbTable::loadLevelDb(const char* lexNumDenFile)
{
    if(db != NULL)
    {
        delete db;
        db = NULL;
    }

    dbName = ((std::string) lexNumDenFile) + ldbExtension;

    std::cerr << "Loading lexnd in LevelDB format from DB in " << dbName << std::endl;

    // Configure LevelDB parameters
    leveldb::Options loadOptions = options;
    loadOptions.create_if_missing = false;

    leveldb::Status status = leveldb::DB::Open(loadOptions, dbName, &db);

    if (status.ok())
    {
        return THOT_OK;
    }
    else
    {
        std::cerr << "Cannot open DB: " << status.ToString() << std::endl;

        return THOT_ERROR;
    }
}

//-------------------------
bool IncrLexLevelDbTable::print(const char* lexNumDenFile)
{
#ifdef THOT_ENABLE_LOAD_PRINT_TEXTPARS
    return printPlainText(lexNumDenFile);
#else
    return printBin(lexNumDenFile);
#endif
}

//-------------------------
bool IncrLexLevelDbTable::printBin(const char* lexNumDenFile)
{
    bool found;
    std::ofstream outF;
    const std::string prefixFile = ((std::string) lexNumDenFile);
    const std::string binFile = prefixFile + defaultExtension;

    outF.open(binFile.c_str(), std::ios::out);

    if(!outF)
    {
        std::cerr << "Error while printing lexical nd file." << std::endl;

        return THOT_ERROR;
    }
    else
    {
        // Print file with lexical nd values
        // Disable cache for bulk load
        leveldb::ReadOptions options;
        options.fill_cache = false;

        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions(options));

        for(it->SeekToFirst(); it->Valid(); it->Next()) {
            std::vector<WordIndex> vec = keyToVector(it->key().ToString());

            // Do not print entries with only source phrase
            if (vec.size() == 2)
            {
                // (s, t) pair is stored in reversed order in the DB
                WordIndex s = vec[1];
                WordIndex t = vec[0];
                // Print data in the following format: s t numerator denominator
                outF.write((char*) &s, sizeof(WordIndex));
                outF.write((char*) &t, sizeof(WordIndex));
                // Numerator
                float numer;
                stringToFloat(it->value().ToString(), numer);
                outF.write((char*) &numer, sizeof(float));
                // Denominator - get value for s
                float denom = getLexDenom(vec[1], found);
                outF.write((char*) &denom, sizeof(float));
            }
        }

        delete it;

        return THOT_OK;
    }
}

//-------------------------
bool IncrLexLevelDbTable::printPlainText(const char* lexNumDenFile)
{
    bool found;
    std::ofstream outF;
    const std::string prefixFile = ((std::string) lexNumDenFile);
    const std::string txtFile = prefixFile + defaultExtension;

    outF.open(txtFile.c_str(), std::ios::out);

    if(!outF)
    {
        std::cerr << "Error while printing lexical nd file." << std::endl;

        return THOT_ERROR;
    }
    else
    {
        // Print file with lexical nd values
        // Disable cache for bulk load
        leveldb::ReadOptions options;
        options.fill_cache = false;

        leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions(options));

        for(it->SeekToFirst(); it->Valid(); it->Next()) {
            std::vector<WordIndex> vec = keyToVector(it->key().ToString());

            // Do not print entries with only source phrase
            if (vec.size() == 2)
            {
                // (s, t) pair is stored in reversed order in the DB
                WordIndex s = vec[1];
                WordIndex t = vec[0];
                // Print data in the following format: s t numerator denominator
                outF << s << " " << t << " ";
                // Numerator
                float numer;
                stringToFloat(it->value().ToString(), numer);
                outF << numer << " ";
                // Denominator - get value for s
                outF << getLexDenom(vec[1], found) << std::endl;
            }
        }

        delete it;

        return THOT_OK;
    }
}

//-------------------------
void IncrLexLevelDbTable::clear(void)
{
    if(dbName.size() > 0)  // Database name is mandatory
    {
        // Remove old database
        bool dropStatus = drop();

        if(dropStatus == THOT_ERROR)
        {
            exit(2);
        }

        // Create empty database
        leveldb::Status status = leveldb::DB::Open(options, dbName, &db);

        if(!status.ok())
        {
            std::cerr << "Cannot create new levelDB in " << dbName << std::endl;
            std::cerr << "Returned status: " << status.ToString() << std::endl;
            exit(3);
        }
    }
}

//-------------------------
IncrLexLevelDbTable::~IncrLexLevelDbTable(void)
{
    if(db != NULL)
        delete db;

    if(options.filter_policy != NULL)
        delete options.filter_policy;

    if(options.block_cache != NULL)
        delete options.block_cache;
}
