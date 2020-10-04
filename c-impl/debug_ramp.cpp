
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
using namespace std;

class DataItem{
    public: 
        string value; 
        int timestamp; 
        vector<int> txn_keys; 
        DataItem(string a, int b, vector<int> c){
            value = a; 
            timestamp = b; 
            txn_keys = c; 
        }
}; 

int main(){
    vector<int> vec; 
    for (int i=0; i<=6; i++){
        vec.push_back(i); 
    }

    string s = "string"; 
    DataItem data(s, 45, vec); 
    std::cout<<data.value<<std::endl;
}
