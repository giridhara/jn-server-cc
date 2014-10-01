/*
 * BestEffortLongFile.h
 *
 *  Created on: Sep 10, 2014
 *      Author: psarda
 */

#ifndef BESTEFFORTLONGFILE_H_
#define BESTEFFORTLONGFILE_H_

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include "Logger.h"

using namespace std;

namespace JournalServiceServer {

/**
 * Class that represents a file on disk which stores a single <code>long</code>
 * value, but does not make any effort to make it truly durable. This is in
 * contrast to {@link PersistentLongFile} which fsync()s the value on every
 * change.
 *
 * This should be used for values which are updated frequently (such that
 * performance is important) and not required to be up-to-date for correctness.
 */
class BestEffortLongFile {

private:
    string filename;
    long defaultVal;

    long value;
    bool opened;
    ofstream ostrm;

public:

    BestEffortLongFile() :
        filename(),
        defaultVal(-1),
        value(-1),
        opened(false)
    {}

    BestEffortLongFile(string file, long defaultVal)
        :
        filename(file),
        defaultVal(defaultVal),
        ostrm()
    {
        opened = false;
        value = -1;
    }
    ~BestEffortLongFile(){
        if(ostrm.is_open()) {
            ostrm.close();
        }
    }

    int get(long& val) {
        if(lazyOpen() != 0) {
            return -1;
        }
        val = value;
        return 0;
    }

    int set(long newVal) {
        if(lazyOpen() != 0) {
            return -1;
        }
        ostrm.seekp(0);
        ostringstream sstrm;
        sstrm << setfill('0') << setw(19) << newVal;
        ostrm << sstrm.str();
        ostrm.flush();
        value = newVal;
        return 0;
    }
    int lazyOpen() {
        if (opened) {
          return 0;
        }

        ifstream ifs(filename.c_str());
        if(!ifs.is_open()) {
            // file does not exist , hence using default value
            value = defaultVal;
        } else {
            long readval;
            char line[128];
            ifs.getline(line, 128);

            if(ifs.good())    // If a line was successfully read
            {
                //done reading from stream, hence closing it
                ifs.close();
                if(strlen(line) == 0)  // Skip any blank lines
                    value = defaultVal;
                else{
                    if(strlen(line) != 19) {
                        LOG.error("File %s had invalid length:", filename.c_str());
                        return -1;
                    }
                    char *end;
                    readval = strtol(line, &end, 10);
                    if(end != line+19)
                        return -1;
                    value = readval;
                }
            }else{
                ifs.close();
                value= defaultVal;
            }
        }

       // Now open file for future writes.
       ostrm.open(filename.c_str());
       if(!ostrm.is_open())
           return -1;
       opened = true;
       return 0;
    }

    int close() {
        if(ostrm.is_open()) {
            ostrm.close();
            if(ostrm.fail()){
                LOG.error("could not close the stream associated with file %s", filename.c_str());
                return -1;
            }
        }
        return 0;
    }
};

}

#endif /* BESTEFFORTLONGFILE_H_ */
