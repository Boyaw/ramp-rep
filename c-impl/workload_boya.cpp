// #include <assert.h>             // Needed for assert() macro
// #include <stdio.h>              // Needed for printf()
// #include <stdlib.h>             // Needed for exit() and ato*()
// #include <math.h>               // Needed for pow()
#include <iostream>
#include <cstdint>
#include <string>
#include <atomic>
#include <cassert>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <exception>
#include <random>
#include <bits/stdc++.h>

#include <typeinfo>


class UniformGenerator{
	public:
		int Next(); 
		int Last(); 
		UniformGenerator(){}
}; 


double RandomDouble(double min = 0.0, double max = 1.0) {
  static std::default_random_engine generator;
  static std::uniform_real_distribution<double> uniform(min, max);
  return uniform(generator);
}

class ZipfianGenerator{
	public: 
		int Next(int num){
			assert(num >= 2 && num < kMaxNumItems);
			std::lock_guard<std::mutex> lock(mutex_);

			if (num>n_for_zeta_){
				RaiseZeta(num); 
				eta_=Eta(); 
			}

			double u = RandomDouble(); 
			double uz = u*zeta_n_; 

			if (uz<1.0){
				return last_value_=base_+0; 
			}

			if (uz < 1.0+std::pow(0.5, theta_)){
				return last_value_=base_+1; 
			}
			last_value_ = base_ + num * std::pow(eta_ * u - eta_ + 1, alpha_);
			return last_value_; 
		}; 
		
		int Next(){ return Next(num_items_); }; 
		
		int Last(){
			std::lock_guard<std::mutex> lock(mutex_); 
			return last_value_; 
		}; 
		
		double kZipfianConst = 0.99; 
		int kMaxNumItems = INT_MAX; 

		// zipfian_constant will always be kZipfianConst
		ZipfianGenerator(int min, int max):
		num_items_(max - min + 1), base_(min), theta_(kZipfianConst),
      zeta_n_(0), n_for_zeta_(0){

			assert(num_items_>=2 && num_items_<kMaxNumItems); 
			zeta_2_ = Zeta(2, theta_);
    		alpha_ = 1.0 / (1.0 - theta_);
    		RaiseZeta(num_items_);
    		eta_ = Eta();   
    		Next();
		}

		ZipfianGenerator(int num_i):ZipfianGenerator(0, num_i){}

	private:
		// Compute the zeta constant needed for the distribution.
  		// Remember the number of items, so if it is changed, we can recompute zeta.
		

		int num_items_; // Number of items
  		int base_; // Min number of items to generate
  		// Computed parameters for generating the distribution
  		double theta_, zeta_n_, eta_, alpha_, zeta_2_;
  		int n_for_zeta_; /// Number of items used to compute zeta_n
  		
		int last_value_; // TODO: what is this?
		std::mutex mutex_;

  		// Calculate the zeta constant needed for a distribution.
  		// Do this incrementally from the last_num of items to the cur_num.
  		// Use the zipfian constant as theta. Remember the new number of items
  		// so that, if it is changed, we can recompute zeta.
 		double Zeta(uint64_t last_num, uint64_t cur_num, double theta, double last_zeta) {
    		double zeta = last_zeta;
    		for (uint64_t i = last_num + 1; i <= cur_num; ++i) {
      			zeta += 1 / std::pow(i, theta);
    		}
    		return zeta;
  		}
  
		double Zeta(uint64_t num, double theta) {
    		return Zeta(0, num, theta, 0);
  		}


  		void RaiseZeta(int num) {
    		assert(num >= n_for_zeta_);
    		zeta_n_ = Zeta(n_for_zeta_, num, theta_, zeta_n_);
    		n_for_zeta_ = num;
  		}
  
  		double Eta() {
    		return (1 - std::pow(2.0 / num_items_, 1 - theta_)) / (1 - zeta_2_ / zeta_n_);
  		}

  
}; 


/*
//=========================================================================
//= Multiplicative LCG for generating uniform(0.0, 1.0) random numbers    =
//=   - x_n = 7^5*x_(n-1)mod(2^31 - 1)                                    =
//=   - With x seeded to 1 the 10000th x value should be 1043618065       =
//=   - From R. Jain, "The Art of Computer Systems Performance Analysis," =
//=     John Wiley & Sons, 1991. (Page 443, Figure 26.2)                  =
//=========================================================================
double rand_val(int seed)
{
  const long  a =      16807;  // Multiplier
  const long  m = 2147483647;  // Modulus
  const long  q =     127773;  // m div a
  const long  r =       2836;  // m mod a
  static long x;               // Random int value
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0)
  {
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + m;

  // Return a random value between 0.0 and 1.0
  return((double) x / m);
}

int zipf(double alpha, int n)
{
  static int first = true;      // Static first time flag
  static double c = 0;          // Normalization constant
  static double *sum_probs;     // Pre-calculated sum of probabilities
  double z;                     // Uniform random number (0 < z < 1)
  int zipf_value;               // Computed exponential value to be returned
  int low, high, mid;           // Binary-search bounds

  // Compute normalization constant on first call only
  if (first == true)
  {
    for (int i=1; i<=n; i++)
      c = c + (1.0 / pow((double) i, alpha));
    c = 1.0 / c;

    sum_probs = (double *)malloc((n+1)*sizeof(*sum_probs));
    sum_probs[0] = 0;
    for (int i=1; i<=n; i++) {
      sum_probs[i] = sum_probs[i-1] + c / pow((double) i, alpha);
    }
    first = false;
  }

  // Pull a uniform random number (0 < z < 1)
  do
  {
    z = rand_val(0);
  }
  while ((z == 0) || (z == 1));

  // Map z to the value
  low = 1, high = n;
  do {
    mid = floor((low+high)/2);
    if (sum_probs[mid] >= z && sum_probs[mid-1] < z) {
    	zipf_value = mid;
    	break;
    } else if (sum_probs[mid] >= z) {
    	high = mid-1;
    } else {
    	low = mid+1;
    }
  } while (low <= high);

  // Assert that zipf_value is between 1 and N
  assert((zipf_value >=1) && (zipf_value <= n));

  return(zipf_value);
}
*/

int main(){
	
	ZipfianGenerator zg(100, 200);
	for (int i=0; i<100; i++){
		std::cout<<"------"<<std::endl;
		// std::cout<<i<<std::endl;
		
		std::cout<<zg.Next()<<std::endl;
		//int zi = zipf(100, 10); 
		//std::cout<<RandomDouble()<<std::endl;
		//std::cout<<typeid(RandomDouble()).name()<<std::endl;
	}
	 
	return 0; 
	
}
 