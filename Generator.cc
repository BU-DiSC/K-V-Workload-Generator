#include "Generator.h"
#include "math.h"
#include <algorithm>
#include "time.h"

int myrandom (int i) { return std::rand()%i;}

Generator::Generator(){}
Generator::Generator(int dist, uint32_t lb, uint32_t ub, double norm_mean, double norm_stddev, double beta_alpha, double beta_beta, double zipf_alpha, int zipf_size, std::vector<int> _index_mapping): dist_(dist), lb_(lb), ub_(ub), norm_mean_(norm_mean), norm_stddev_(norm_stddev), beta_alpha_(beta_alpha), beta_beta_(beta_beta), zipf_alpha_(zipf_alpha), zipf_size_(zipf_size){
	//gen.seed(std::chrono::system_clock::now().time_since_epoch().count());
	distribution0 = std::uniform_int_distribution<int>(lb_, ub_);
	distribution1 = std::normal_distribution<double>(norm_mean_, norm_stddev_);
	distribution2_x = std::gamma_distribution<double>(beta_alpha_, 1.0);
	distribution2_y = std::gamma_distribution<double>(beta_beta_, 1.0);


        //for zipfian distribution
        if(dist == 3){
            zipf_normalize_constant = 0;
            for(int i = 1; i < zipf_size_; i++){
                zipf_normalize_constant += 1/(pow((double)i, zipf_alpha_));
            }
	    if(_index_mapping.size() == 0 ){
                index_mapping = std::vector<int> (zipf_size_, 0);
			
                for(int i = 1; i < zipf_size_; i++){
                    index_mapping[i] = i;
		}
            }else{
		index_mapping = _index_mapping;
		int size = _index_mapping.size();
		while(size < zipf_size_){
                    int pos = rand()%size;
		    index_mapping.insert(index_mapping.begin() + pos, size);
                    size++;
                }
            }
            cumulative_probabilities = std::vector<double> ( zipf_size+1, 0.0);
            cumulative_probabilities[0] = 0.0;
            for(int i = 1; i < zipf_size_; i++){
                cumulative_probabilities[i] = cumulative_probabilities[i-1] + 1/(pow((double)i, zipf_alpha_)*zipf_normalize_constant);
            }
            cumulative_probabilities[zipf_size_] = 1.0;
            std::random_shuffle(index_mapping.begin(), index_mapping.end(), myrandom); 
        }else{
            /*
            cumulative_probabilities = std::vector<double> ( zipf_size+1, 0.0);
            index_mapping = std::vector<int> (zipf_size_, 0);
            if(zipf_size_ == 0){
                zipf_size_ = 1;
            }
            double unit = 1/zipf_size_;
            for(int i = 0; i < zipf_size; i++){
                cumulative_probabilities[i+1] = cumulative_probabilities[i] + unit;
		index_mapping[i] = i;
            }*/
            cumulative_probabilities = std::vector<double> ();
        }
        uniform_standard_distribution = std::uniform_real_distribution<double>(0.0,1.0);
}

uint32_t Generator::getNext(){
	switch (dist_) { // 0 -> uniform; 1 -> norm; 2 -> beta; 3-> Zipf
		case 0: {
			uint32_t number = (uint32_t) distribution0(gen);
			while(number < lb_ || number > ub_){
				number = distribution0(gen);
			}	
			return number;
		}
		case 1: {
			uint32_t number = (uint32_t) round(distribution1(gen));
			while(number < lb_ || number > ub_){
				number = (uint32_t) round(distribution1(gen));
			}	
			return number;
		}
		case 2: {
			double X = distribution2_x(gen);
			double Y = distribution2_y(gen);
			return lb_ + round((ub_ - lb_) * (X/(X+ Y)));
		}
                case 3: {
                        double p = uniform_standard_distribution(gen);
                        while(p == 0 || p == 1) p = uniform_standard_distribution(gen);
                        // binary search
                        int low = 0;
                        int high = zipf_size_-1;
                        int mid = (low + high)/2;
                        while(high - low > 1){
                            //std::cout << "p: " << p << "\tmid: " << cumulative_probabilities[mid] << "\tmid+1: " << cumulative_probabilities[mid+1] << std::endl;
                            if(p < cumulative_probabilities[mid]){
                                high = mid;
                            }else if(p >= cumulative_probabilities[mid+1]){
                                low = mid; 
                            }else{
                                break;
                            }
                            mid = (low+high)/2;
                        }
                        return index_mapping[mid]; 

                }
		default:{
			std::cout << "Unexpected case" << std::endl;
			return 0;
		}

	}
}
