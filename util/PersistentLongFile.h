/*
 * PersistentLongFile.h
 *
 *  Created on: Sep 5, 2014
 *      Author: psarda
 */

#ifndef PERSISTENTLONGFILE_H_
#define PERSISTENTLONGFILE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "JNServiceMiscUtils.h"

using namespace std;

namespace JournalServiceServer {

class PersistentLongFile {
    private:
    string file;
    long defaultVal;

    long value;
    bool loaded;

    public :
    PersistentLongFile() :
        file(),
        defaultVal(-1),
        value(-1),
        loaded(false)
    {}

    PersistentLongFile(const string& file, const long defaultVal)
        :
        file(file),
        defaultVal(defaultVal),
        value(defaultVal),
        loaded(false)
    {}

    int get(long& result) {
        if (!loaded) {
            long temp;
            int status = readFile(file, defaultVal, temp);
            if( status != 0)
                return status;
            value = temp;
            loaded = true;
        }
        result = value;
        return 0;
    }

    int set(const long newVal) {
        if (value != newVal || !loaded) {
            if(writeFile(file, newVal) != 0)
                return -1;
        }
        value = newVal;
        loaded = true;
        return 0;
    }

    /**
    * Atomically write the given value to the given file, including fsyncing.
    *
    * @param file destination file
    * @param val value to write
    * @throws IOException if the file cannot be written
    */
    int writeFile(string file, long val) const {
    ofstream fos(file.c_str());
        if (!fos.is_open())
            return -1;
        fos << dec;
        fos << val << "\n";
        fos.close();
        return 0;
    }

    int readFile(const string& file, const long& defaultVal, long &result) const {
        long val = defaultVal;

        bool file_exists_flag = false;

        if(file_exists(file, file_exists_flag) != 0){
            return -1;
        }

        if (!file_exists_flag) {
            result = val;
            return 0;
        }
        ifstream ifs (file.c_str());
        if(!ifs.is_open()) {
            return -1;
        }
        string line;
        if (!getline(ifs,line)) {
            ifs.close();
            return -1;
        }
        ifs.close();
        char *end;
        const char *cstring = line.c_str();
        val = strtol(cstring, &end, 10);
        if(end != cstring+line.length())
            return -1;
        result = val;
        return 0;
    }
};

}

#endif /* PERSISTENTLONGFILE_H_ */
