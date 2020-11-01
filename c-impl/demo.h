// Header
#include <iostream>
#include <random> 
#include <map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional> //for std::function
#include <experimental/algorithm> 
#include <string>
#include <iterator>
#include <semaphore.h>
using namespace std;



class DataItem{
    public: 
        string value; 
        int timestamp; 
        vector<int> txn_keys; 
        // Add default DataItem constructor
        DataItem(){
            value = {}; 
            timestamp = 0; 
            txn_keys = {}; 
        }

        DataItem(string a, int b, vector<int> c){
            value = a; 
            timestamp = b; 
            txn_keys = c; 
        }; 
}; 

