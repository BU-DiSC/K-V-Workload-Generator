#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>
#include <iostream>
#include <chrono>
#include <vector>

class Generator{
	int dist_; // 0 -> unifrom; 1 -> norm; 2 -> beta; 3 -> Zipf
	uint32_t lb_;
	uint32_t ub_;
	double norm_mean_;
	double norm_stddev_;
	double beta_alpha_;
	double beta_beta_;	
        double zipf_alpha_;
        int zipf_size_;
        double zipf_normalize_constant;
	std::default_random_engine gen;
	std::uniform_int_distribution<int> distribution0;
	std::normal_distribution<double> distribution1;
	std::gamma_distribution<double> distribution2_x;
	std::gamma_distribution<double> distribution2_y;

        // for zipf distribution
	std::uniform_real_distribution<double> uniform_standard_distribution; 
        std::vector<double> cumulative_probabilities;
	
public:
        std::vector<int> index_mapping; 
	Generator();
	Generator(int dist, uint32_t lb, uint32_t ub, double norm_mean, double norm_stddev, double beta_alpha, double beta_beta, double zipf_alpha, int zipf_size, std::vector<int> _index_mapping = {});
	uint32_t getNext();
};
#endif
