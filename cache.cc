#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>

using namespace std;

void printUsage() {
    // nk: the capacity of the cache in kilobytes (an int)
    // assoc: the associativity of the cache (an int)
    // blocksize: the size of a single cache block in bytes (an int)
    // repl: the replacement policy (a char); 'l' means LRU, 'r' means random.
    cout << "Usage: ./cache {nk} {assoc} {blocksize} {repl}" << endl;
    cout << "nk: the capacity of the cache in kilobytes (an int)\n";
    cout << "assoc: the associativity of the cache (an int)\n";
    cout << "blocksize: the size of a single cache block in bytes (an int)\n";
    cout << "repl: the replacement policy (a char); 'l' means LRU, 'r' means random.\n";
}

void printWRError(const string& rwErr){
    cout << "Issue reading commands, make sure command is \"r/w addr\"\n";
    cout << "cache error on command: "<<rwErr<<endl;
}

string hex2bin(string hex){
    string bin;

    for(int i=0;i< 16-hex.length();i++){
        bin += "0000";
    }

    for(int i=0;i<hex.length();i++){
        char c = hex[i];

        switch(tolower(c)){
            case '0':
                bin += "0000";
                break;
            case '1':
                bin += "0001";
                break;
            case '2':
                bin += "0010";
                break;
            case '3':
                bin += "0011";
                break;
            case '4':
                bin += "0100";
                break;
            case '5':
                bin += "0101";
                break;
            case '6':
                bin += "0110";
                break;
            case '7':
                bin += "0111";
                break;
            case '8':
                bin += "1000";
                break;
            case '9':
                bin += "1001";
                break;
            case 'a':
                bin += "1010";
                break;
            case 'b':
                bin += "1011";
                break;
            case 'c':
                bin += "1100";
                break;
            case 'd':
                bin += "1101";
                break;
            case 'e':
                bin += "1110";
                break;
            case 'f':
                bin += "1111";
                break;
        }
    }

    return bin;
}

struct tableEntry{
    bool valid;
    long long int tag;
    int recent;

    tableEntry(){
        valid = false;
        tag = 0;
        recent = 0;
    }

    tableEntry(bool _v, long long int _t){
        valid = _v;
        tag = _t;
        recent = 0;
    }

    void print() const{
        printf("[valid: %d, tag: %lli]", valid, tag);
    }
};

class cache{
private:
    //cache initializers
    int nk; //(in kilobytes)
    int assoc;
    int bs;
    char repl;
    //address size values
    int block_offset;
    int index;
    int tag;
    //book-keeping
    int numReads= 0;
    int numWrites= 0;
    int readMiss= 0;
    int writeMiss= 0;

    vector<tableEntry*> lut;

public:
    cache(int _nk, int _assoc, int _bs, char _repl){
        nk = _nk;
        assoc = _assoc;
        bs = _bs;
        repl = _repl;

        block_offset = (int)log2((double)bs);
        index = (int)log2((nk*1024)/(bs*assoc));
        tag = 64 - block_offset - index;

        int cacheEntrySize = (nk * 1024) / bs;
        for(int i = 0; i < cacheEntrySize; i++){
            lut.push_back(new tableEntry());
        }
        cout<<"tag:"<<tag;
        cout<<" index:"<<index;
        cout<<" block_offset:"<<block_offset<<endl;
    }

    vector<long long int> formatAddr(string addr){
        //inputs binary string -> outputs vector
        vector<long long int> a; //group to contain base 10 values for [tag, index, block offset]
        string bin_block_offset = addr.substr(addr.size()-block_offset,block_offset);
        string bin_index = addr.substr(addr.size()-index-block_offset,index);
        string bin_tag = addr.substr(0,addr.size()-index-block_offset);
        //printf("[%s,%s,%s]\n",bin_tag.c_str(),bin_index.c_str(),bin_block_offset.c_str());
        a.push_back(stoi(bin_tag, nullptr,2));
        a.push_back(stoi(bin_index,nullptr,2));
        a.push_back(stoi(bin_block_offset, nullptr,2));
        //printf("[%lli,%lli,%lli]\n",a[0],a[1],a[2]);
        return a;
    }

    void printStats(){
        // The total number of misses,
        // The percentage of misses (i.e. total misses divided by total accesses),
        // The total number of read misses,
        // The percentage of read misses (i.e. total read misses divided by total read accesses),
        // The total number of write misses,
        // The percentage of write misses (i.e. total write misses divided by total write accesses).
        int totalAccesses = numReads + numWrites;
        int totalMiss = readMiss + writeMiss;
        double missRate = (double)totalMiss/totalAccesses*100;

        printf("%i %f%% %i %f%% %i %f%%\n",totalMiss,missRate,readMiss,(double)readMiss/numReads*100,writeMiss,(double)writeMiss/numWrites*100);
    }

    void read(string addr){
        numReads++;
        //cout<<"read:\""<<addr<<"\""<<endl;
        string bin = hex2bin(addr);
        //cout<<bin<<endl;
        vector<long long int> addrv = formatAddr(bin);

        if(!searchCache(addrv)){

            readMiss++;
            handleMiss(addrv);
        }
    }

    void write(string addr){
        numWrites++;
        //cout<<"write:\""<<addr<<"\""<<endl;
        string bin = hex2bin(addr);
        //cout<<bin<<endl;
        vector<long long int> addrv = formatAddr(bin);

        if(!searchCache(addrv)){
            // if false then handle miss
            writeMiss++;
            handleMiss(addrv);
        }
    }

    bool searchCache(vector<long long int> addr){
        //search if found then true
        //if not found return false and handle miss.
        long long int start = addr[1] * assoc;
        bool found = 0;

        for(long long int i = start;i<start+assoc;i++){
            //lut[i]->print();
            lut[i]->recent++;
            if(lut[i]->valid && lut[i]->tag == addr[0]){
                found = 1;
                lut[i]->recent = 0;
            }
        }
        return found;

    }

    void handleMiss(vector<long long int> addr){
        //cout<<"MISS!!!! -> Handling"<<endl;
        //on miss need to look for open space first with valid bit 0; if so then flip to 1 and put tag in.
        long long int start = addr[1] * assoc;
        //first check for open spot
        for(long long int i = start;i<start+assoc;i++){
           if(!lut[i]->valid){
               lut[i]->valid = true;
               lut[i]->recent = 0;
               lut[i]->tag = addr[0];
               //if placed then exit/ return
               return;
           }
        }
        //if iterate through entire assoc and not placed then must either lru or rand
        if(repl == 'r'){
            //do random replacement
            long long int indexToDie = rand()%assoc;
            indexToDie += start;

            lut[indexToDie]->valid = true;
            lut[indexToDie]->recent = 0;
            lut[indexToDie]->tag = addr[0];

        }
        else if(repl == 'l'){
            //do lru replacement
            long long bv = start;
            for(long long int i = start;i<start+assoc;i++) {
                if(lut[i]->recent > lut[bv]->recent){
                    bv = i;
                }
            }

            lut[bv]->valid = true;
            lut[bv]->recent = 0;
            lut[bv]->tag = addr[0];

        }
        else{
            printf("invalid replacement type of value: %c\n Please use either 'l' -> (lru) or 'r' -> (random)",repl);
            exit(-1);
        }

        return;
    }

};

int main(int argc,char** argv){
    //g++ -o cache cache.cc
    //gzip -dc 429.mcf-184B.trace.txt.gz | ./cache 2048 64 64 l
    //check for correct amount of parameters
    //if not correct print correct usage and exit
    if(argc != 5){
        printUsage();
        exit(-1);
    }
    int nk = stoi(argv[1]);
    int assoc = stoi(argv[2]);
    int blocksize = stoi(argv[3]);
    char repl = argv[4][0];

    //test inputted vals
    printf("nk: %i assoc: %i blocksize: %i repl: %c\n",nk,assoc,blocksize,repl);

    //create our cache to simulate
    cache simCache = cache(nk,assoc,blocksize,repl);

    //now decode input from pipe and call read write in cache for all exchanges
    string s;
    vector<string> input;
    while(getline(cin,s)){
        if(s.empty()){
            break;
        }
        input.push_back(s);
    }

    for(auto & i : input){
        //cout<<input[i]<<endl;
        //input format -> "op addr"
        string op = i.substr(0,i.find(" "));
        string addr = i.substr(i.find(" ")+1);
        //checks if instr is r
        if( op == "r"){
            simCache.read(addr);
        }
        else if( op == "w"){
            simCache.write(addr);
        }
        else{
            printWRError(i);
            exit(-1);
        }
    }

    simCache.printStats();

    return 0;
}