
#include "demo.h"
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
        DataDict versions={}; 
        Dict lastcommit={}; 
        sem_t p_sem; 
        Partition(){
            sem_init(&p_sem, 0, 1); 
        }
        


        // 2pc for writing: prepare + commit
        void prepare(int key, DataItem value, int timestamp){
            versions[std::make_pair(key, timestamp)] = value;
        } 

        void commit(int key, int timestamp){
            // add lock guard here to make sure each lastcommit succeeds for synchornization
            sem_wait(&p_sem); 
            if(lastcommit[key] < timestamp){
                lastcommit[key] = timestamp; 
            } 
            sem_post(&p_sem); 
        } 

        DataItem getRAMPFast(int key, int ts_required){ 
            // ts_required is initialized as 0
            if(ts_required == 0){
                return versions[std::make_pair(key, lastcommit[key])]; 
            }
            else{
                return versions[std::make_pair(key, ts_required)]; 
            }
        }
};


// Client Class
// without implementing hash bloomfilter

class Client{
    public: 
        int id; 
        int sequence_number = 0; 
        vector <Partition> partitions; 
        int algorithm; 
        Client(int a, vector <Partition> b, int c){
            id = a; 
            partitions = b; 
            algorithm = c; 
        }; 

        Partition key_to_partition(int key){
            // TODO hash function here
            int map_par = key%size(partitions); 
            return partitions[map_par]; 
        };

        int next_timestamp(){
            // not like pyimp using shift
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

                // TODO Do I need unordered map here?
                std::map<int,int> vlatest = {};
                for(std::map<int, DataItem>::iterator i = results.begin(); i != results.end(); ++i){
                    // TODO is it possiblt to have value = none? 
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

// int NUM_TXNS = 1000; initiate with small number of transactions
int NUM_TXNS = 100; 
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

sem_t mysemaphore; 

void run_client(Client c, vector<int> all_keys){


    while(!(sem_wait(&mysemaphore))){
        std::cout<<"we are in while loop for client "+to_string(c.id)<<std::endl;
        
        //generate some keys
        vector<int> txn_keys; 
        std::experimental::sample(all_keys.begin(), all_keys.end(), std::back_inserter(txn_keys),
                TXN_LENGTH, std::mt19937{std::random_device{}()}); 


        double r = ((double) rand() / (RAND_MAX)); 
        if(r < READ_PROPORTION){
            std::cout<<"this time we read "+to_string(c.id)<<std::endl; 
            c.get_all_items(txn_keys); 
            std::cout<<"get_all_items succeed "+to_string(c.id)<<std::endl; 
        }
        else{
            std::cout<<"this time we write "+to_string(c.id)<<std::endl; 
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
            std::cout<<"write succeed "+to_string(c.id)<<std::endl; 
        } 
    }
}



int main(){
    vector <int> KEYS = key_generator(NUM_KEYS); 
    vector <Partition> PARTITIONS = partition_generator(NUM_PARTITIONS); 

    sem_init(&mysemaphore, 0, NUM_TXNS); 
    std::vector<std::thread> grp;
    Client client1(1, PARTITIONS, ALGORITHM); 
    std::thread t1(run_client, client1, KEYS);  
    Client client2(2, PARTITIONS, ALGORITHM); 
    std::thread t2(run_client, client2, KEYS);
    Client client3(3, PARTITIONS, ALGORITHM); 
    std::thread t3(run_client, client3, KEYS);
    Client client4(4, PARTITIONS, ALGORITHM); 
    std::thread t4(run_client, client4, KEYS);
    Client client5(5, PARTITIONS, ALGORITHM); 
    std::thread t5(run_client, client5, KEYS);


    t1.join(); 
    std::cout<<"t1 done!"<<std::endl; 
    t2.join(); 
    std::cout<<"t2 done!"<<std::endl; 
    t3.join(); 
    std::cout<<"t3 done!"<<std::endl; 
    t4.join(); 
    std::cout<<"t4 done!"<<std::endl; 
    t5.join(); 
    std::cout<<"t5 done!"<<std::endl; 

    /*

    for (int c_id=0; c_id<NUM_CLIENTS; c_id++){
        Client client(c_id, PARTITIONS, ALGORITHM); 
        grp.push_back(std::thread(run_client, client, KEYS));  
    } 

    std::cout<<"Vector succeed!"<<std::endl; 

    // Join all
    for(auto &t : grp){
        t.join(); 
    }
    */

    std::cout<<"done!"<<std::endl; 
    return 0; 
}


