
#include <demo.h>
using namespace std;

// DataDict is a map from [key, timestamp] to DataItem
typedef std::map<std::pair<int, int>, DataItem> DataDict; 
// Dict is a map from key to the latest commited timestamp
typedef std::map<int, int> Dict; 
// StrDict is a map from key to value string
typedef std::map<int, string> StrDict; 
 


// Server Class
class RAMPAlgorithm{
    public:
        int Fast = 0; 
        int Small = 1; // TODO: just realize Fast algorithm first
}; 

class Partition{
    public: 
        DataDict versions = {}; 
        Dict lastcommit = {}; 

    // 2pc for writing: prepare + commit
    void prepare(int key, DataItem value, int timestamp){
        versions[std::make_pair(key, timestamp)] = value;
    }; 

    void commit(int key, int timestamp){
        // TODO add lock here
        if(lastcommit[key] < timestamp){
            lastcommit[key] = timestamp; 
        }; 
    };

    DataItem getRAMPFast(int key, int ts_required){ //TODO ts_required type might be wrong
        if(ts_required == 0){
            return versions[std::make_pair(key, lastcommit[key])]; 
        }
        else{
            return versions[std::make_pair(key, ts_required)]; 
        }
    };
};


// Client Class
// without implementing hash bloomfilter

class Client{
    public: 
        int id; 
        int sequence_number = 0; 
        // TODO define Partitions type
        vector <Partition> partitions; 
        int algorithm; 
        Client(int a, vector <Partition> b, int c){
            this->id = a; 
            this->partitions = b; 
            this->algorithm = c; 
        }; 

        Partition key_to_partition(int key){
            // TODO hash function here
            return partitions[key]; 
        };

        int next_timestamp(){
            // sequence number why using shift
            sequence_number += 1; 
            return sequence_number; 
        }; 

        void put_all(StrDict kvps){
            int timestamp = next_timestamp(); 

            vector <int> txn_keys; 
            // 0 means RAMPFast
            if (this->algorithm == 0){
                for(StrDict::iterator it = kvps.begin(); it != kvps.end(); ++it){
                    txn_keys.push_back(it->first); 
                }; 
            }; 

            for(StrDict::iterator ite = kvps.begin(); ite != kvps.end(); ++ite){ 
                DataItem newItem(ite->second, timestamp, txn_keys); 
                key_to_partition(ite->first).prepare(ite->first, newItem, timestamp); 
            }; 

            for(StrDict::iterator ite = kvps.begin(); ite != kvps.end(); ++ite){ 
                key_to_partition(ite->first).commit(ite->first, timestamp); 
            }; 
        };

        StrDict get_all_items(vector <int> keys){
            std::map<int, DataItem> results;
            //0 means RAMPFast
            if(algorithm == 0){                 
                for(int ele : keys){
                    results.insert({ele,key_to_partition(ele).getRAMPFast(ele, 0)}); 
                }

                // Do I need unordered map here?
                std::map<int,int> vlatest = {};
                for(std::map<int, DataItem>::iterator i = results.begin(); i != results.end(); ++i){
                    // is it possiblt to have value = none? 
                    for(int j : i->second.txn_keys){
                        if(vlatest[j] < i->second.timestamp){
                            vlatest[j] = i->second.timestamp; 
                        }
                    } 
                } 

                for(int ele : keys){
                    if(results.count(ele)==1 && (results[ele].value=="" || results[ele].timestamp < vlatest[ele])){
                        results[ele] = key_to_partition(ele).getRAMPFast(ele, vlatest[ele]); 
                    }
                } 

            }

            std::map<int, string> final_results; 

            for(std::map<int, DataItem>::iterator i = results.begin(); i != results.end(); ++i){
                final_results.insert({i->first, i->second.value});  
            }

            return final_results; 


        } 


};


//ALGORITHM = RAMPAlgorithm.Fast
RAMPAlgorithm Ramp; 
int ALGORITHM = Ramp.Fast; 
int NUM_PARTITIONS = 5; 
int NUM_CLIENTS = 5; 

int NUM_TXNS = 1000; 
float READ_PROPORTION = 0.5; 
int TXN_LENGTH = 4; 
int NUM_KEYS = 100; 

vector <int> key_generator(int number_key){
    vector <int> KEYS; 
    for(int i = NUM_KEYS; i > 0; i--){
        KEYS.insert(KEYS.begin(), i);
    }
    return KEYS; 
}

vector <Partition> partition_generator(int number_part){
    vector <Partition> PARTITIONS; 
    for(int i = NUM_PARTITIONS; i>0; i--){
        Partition one_partition; 
        PARTITIONS.insert(PARTITIONS.begin(), one_partition); 
    }
    return PARTITIONS; 
} 

Semaphore request_sem(NUM_TXNS); 
Semaphore finished_sem(0); 

// generate random string
typedef std::vector<char> char_array;

char_array charset()
{
    //Change this to suit
    return char_array( 
    {
    'A','B','C','D','E','F',
    'G','H','I','J','K',
    'L','M','N','O','P',
    'Q','R','S','T','U',
    'V','W','X','Y','Z'
    });
};    

// given a function that generates a random character,
// return a string of the requested length
std::string random_string( size_t length, std::function<char(void)> rand_char )
{
    std::string str(length,0);
    std::generate_n( str.begin(), length, rand_char );
    return str;
}


void run_client(Client c){
    while(request_sem.read_count()){
        request_sem.wait(c.id);
        
        //TODOnow generate some keys
        vector <int> txn_keys; 
        double r = ((double) rand() / (RAND_MAX)); 
        if(r < READ_PROPORTION){
            c.get_all_items(txn_keys); 
        }
        else{
            StrDict kvps; 

            // generate random string here
            const auto ch_set = charset();
            std::default_random_engine rng(std::random_device{}());
            std::uniform_int_distribution<> dist(0, ch_set.size()-1);
            auto randchar = [ ch_set,&dist,&rng ](){return ch_set[ dist(rng) ];};
            string value = random_string(6,randchar); 
 
            for (int i : txn_keys){
                kvps.insert({i, value}); 
            }
            c.put_all(kvps); 
        }
        // cid just for internl output, maybe useful for debug
        finished_sem.notify(c.id); 
    }
}



int main(){
    vector <int> KEYS = key_generator(NUM_KEYS); 
    vector <Partition> PARTITIONS = partition_generator(NUM_PARTITIONS);
    for (int c_id=0; c_id<NUM_CLIENTS; c_id++){
        Client client(c_id, PARTITIONS, ALGORITHM); 
        string client_id = to_string(c_id); 
        std::thread client_id (run_client, client); 
    } 

    std::cout<<"done!"<<std::endl; 
    return 0; 
}


