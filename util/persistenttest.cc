#include "PersistentLongFile.h"
int main(int argc, char* argv[]) {
    PersistentLongFile lf("/home/psarda/lf.txt", 100);
//    int ws = lf.set(1729);
//    if(ws == -1) {
//        cout << "error while writing" << "\n";
//        return ws;
//    }
    long readval;
    int rs = lf.get(readval);
    if(rs == 0 )
    	cout << "value read back from file is '" << readval << "'" << endl;
	else
	    cout << "error while get call" << endl;
}
