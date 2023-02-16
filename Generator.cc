#include "Generator.h"
#include "math.h"
#include <algorithm>
#include "time.h"

#define ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS 1000
#define ZIPFIAN_GENERATOR_SMALL_AREA_SIZE 50000

int myrandom (int i) { return std::rand()%i;}

inline int binary_search(double p, int low, int high, std::vector<double> & cdf) {
    //std::cout << "p : " << p << "\tlow : " << low << "\t high : " << high << std::endl;
    int mid = (low + high)/2;
    while(high - low > 1){
       //std::cout << "p: " << p << "\tmid: " << cumulative_probabilities[mid] << "\tmid+1: " << cumulative_probabilities[mid+1] << std::endl;
       if(p < cdf[mid]){
             high = mid;
       }else if(p >= cdf[mid+1]){
             low = mid; 
       }else{
             break;
       }
       mid = (low+high)/2;
    }
    return mid;
}

Generator::Generator(){}
Generator::Generator(int dist, uint32_t lb, uint32_t ub, double norm_mean, double norm_stddev, double beta_alpha, double beta_beta, double zipf_alpha, int zipf_size, std::vector<int> _index_mapping): dist_(dist), lb_(lb), ub_(ub), norm_mean_(norm_mean), norm_stddev_(norm_stddev), beta_alpha_(beta_alpha), beta_beta_(beta_beta), zipf_alpha_(zipf_alpha), zipf_size_(zipf_size){
	gen.seed(std::chrono::system_clock::now().time_since_epoch().count());
	distribution0 = std::uniform_int_distribution<int>(lb_, ub_);
	distribution1 = std::normal_distribution<double>(norm_mean_, norm_stddev_);
	distribution2_x = std::gamma_distribution<double>(beta_alpha_, 1.0);
	distribution2_y = std::gamma_distribution<double>(beta_beta_, 1.0);
	distribution3 = std::uniform_int_distribution<int>(0, ZIPFIAN_GENERATOR_SMALL_AREA_SIZE-1);



        //for zipfian distribution
        if(dist == 3){
	    double zipf_small_normalize_constant = 0;
	    zipf_generator_small_area = std::vector<uint32_t> (ZIPFIAN_GENERATOR_SMALL_AREA_SIZE, 0);
	    std::vector<double>* small_cumulative_probabilities = new std::vector<double> (ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS+1, 0);
            zipf_normalize_constant = 0;
            for(int i = 1; i < zipf_size_; i++){
                zipf_normalize_constant += 1/(pow((double)i, zipf_alpha_));
		if (i == ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS) {
		    zipf_small_normalize_constant = zipf_normalize_constant;
		}
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
	    small_cumulative_probabilities->at(0) = 0.0;
            for(int i = 1; i < zipf_size_; i++){
                cumulative_probabilities[i] = cumulative_probabilities[i-1] + 1/(pow((double)i, zipf_alpha_)*zipf_normalize_constant);
		if (i <= ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS) {
		  small_cumulative_probabilities->at(i) = small_cumulative_probabilities->at(i-1) + 1.0/(pow((double)i, zipf_alpha_)*zipf_small_normalize_constant);
		}
            }
	    zipf_searching_threshold_ = cumulative_probabilities[ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS];
            cumulative_probabilities[zipf_size_] = 1.0;
	    small_cumulative_probabilities->at(ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS) = 1.0;
            std::random_shuffle(index_mapping.begin(), index_mapping.end(), myrandom); 

	    double p;
	    for (uint32_t k = 0; k < ZIPFIAN_GENERATOR_SMALL_AREA_SIZE; k++) {
                p = uniform_standard_distribution(gen);
                while(p == 0 || p == 1) p = uniform_standard_distribution(gen);
		zipf_generator_small_area[k] = binary_search(p, 0, ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS-1, *small_cumulative_probabilities);
	    }
	    std::cout << std::endl;
	    small_cumulative_probabilities->clear();
	    delete small_cumulative_probabilities;
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

			if (p < zipf_searching_threshold_) {
			  return index_mapping[zipf_generator_small_area[distribution3(gen)]];

			} else if (p == zipf_searching_threshold_) {
			  return index_mapping[ZIPFIAN_THRESHOLD_UNIQ_ELEMENTS];
			}
			int step = 1;
			int start = 0;
                        int low = 0;
                        int high = zipf_size_-1;
			for (; start < zipf_size_; start += step) {
			   if (cumulative_probabilities[start] >= p) {
			       low = start > step ? (start - step + 1): 0;
			       high = start;
			       break;
			   } 
			   step *= 2;
			}

			if (start > zipf_size_) {
			    low = start - step;
			    high = zipf_size_ - 1;
			}
			
                        return index_mapping[binary_search(p, low, high, cumulative_probabilities)]; 

                }
		default:{
			std::cout << "Unexpected case" << std::endl;
			return 0;
		}

	}
}
