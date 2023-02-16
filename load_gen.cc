/* *  Created on: September 05, 2019
 *  Author: Subhadeep
 */

// #include <cstdio>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <iomanip>
#include "args.hxx"
#include "Generator.h"
#include "Key.h"

#define U_THRESHOLD 0.1 // U_THRESHOLD*insert_count number of inserts must be made before Updates may take place (applicable when an empty database is being populated)
#define PD_THRESHOLD 0.1 // PD_THRESHOLD*insert_count number of inserts must be made before Point Deletes may take place (applicable when an empty database is being populated)
#define RD_THRESHOLD 0.75 // RD_THRESHOLD*insert_count number of inserts must be made before Range Deletes may take place (applicable when an empty database is being populated)
#define PQ_THRESHOLD 0.1 // PQ_THRESHOLD*insert_count number of inserts must be made before Point Queries may take place (applicable when an empty database is being populated)
#define RQ_THRESHOLD 0.1 // RQ_THRESHOLD*insert_count number of inserts must be made before Range Queries may take place (applicable when an empty database is being populated)
#define STRING_KEY_ENABLED false
#define FILENAME "workload.txt"

// using namespace std;

// temporary global variables -- are to be programmed as commandline args
std::string file_path = ""; 
long insert_count = 0;
long update_count = 0;
long point_delete_count = 0;
long range_delete_count = 0;
float range_delete_selectivity = 0;
long point_query_count = 0;
long range_query_count = 0;
float range_query_selectivity = 0;
float zero_result_point_delete_proportion = 0;
float zero_result_point_lookup_proportion = 0;
long existing_point_query_count = 0;
long non_existing_point_query_count = 0;
long maximum_unique_non_existing_point_query_count = 0;
long maximum_unique_existing_point_query_count = 0;
uint32_t entry_size = 8; // in bytes // size of uint32_t = 4 bytes // range of uint32_t = 0 - 4294967295 (2^32 - 1)
uint32_t key_size = 4;
float lambda = -1; // lambda = key_size / (key_size + value_size) ; key_size = entry_size * lambda ; value_size = entry_size * (1-lambda)
bool load_from_existing_workload = false;
std::string out_filename = "";

const char value_alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; // "0123456789";

std::vector<Key> insert_pool;
std::set<Key> global_insert_pool_set; 
std::vector<Key> global_insert_pool; 
std::set<Key> global_non_existing_key_set;
std::vector<Key> global_non_existing_key_pool;

// distribution params: 0 -> uniform; 1 -> normal; 2 -> gamma
int num_insert_key_prefix = 62*62;
int insert_dist = 0;
float insert_norm_mean_percentile = 0;
float insert_norm_stddev = 0.1;
float insert_beta_alpha = 1.0;
float insert_beta_beta = 1.0;
float insert_zipf_alpha = 1.0;
Generator* insertIndexGenerator = nullptr;
int update_dist = 0;
float update_norm_mean_percentile = 0;
float update_norm_stddev = 0.1;
float update_beta_alpha = 1.0;
float update_beta_beta = 1.0;
float update_zipf_alpha = 1.0;
bool sorted = false;
Generator* updateIndexGenerator = nullptr;
int non_existing_point_lookup_dist = 0;
float non_existing_point_lookup_norm_mean_percentile = 0;
float non_existing_point_lookup_norm_stddev = 0.1;
float non_existing_point_lookup_beta_alpha = 1.0;
float non_existing_point_lookup_beta_beta = 1.0;
float non_existing_point_lookup_zipf_alpha = 1.0;
Generator* nonExistingPointLookupIndexGenerator = nullptr;
int existing_point_lookup_dist = 0;
float existing_point_lookup_norm_mean_percentile = 0;
float existing_point_lookup_norm_stddev = 0.1;
float existing_point_lookup_beta_alpha = 1.0;
float existing_point_lookup_beta_beta = 1.0;
float existing_point_lookup_zipf_alpha = 1.0;
Generator* existingPointLookupIndexGenerator = nullptr;

int parse_arguments2(int argc, char *argv[]);
int get_choice(long, long, long, long, long, long, long, long, long, long, long, long, long);
void generate_workload();
void print_workload_parameters(int _insert_count, int _update_count, int _point_delete_count,int  _range_delete_count,int _effective_ingestion_count);
std::string get_value(int _value_size);
inline void showProgress(const uint32_t &n, const uint32_t &count);

/*
uint32_t get_key_as_uint32_t() { // random number generator still not foolproof
    return (uint32_t) (rand()*rand()) % KEY_DOMAIN;
}

Key get_key(int _key_size){
        //return std::to_string(get_key_as_uint32_t());
    char *s = new char[(int)_key_size];
    for (int i = 0; i < _key_size; ++i) {
        s[i] = key_alphanum[rand() % (sizeof(key_alphanum) - 1)];
    }
    s[_key_size] = '\0';
    return s;
}*/

std::string get_value(int _value_size) {
    // std::cout << key_size << std::endl;
    char *s = new char[(int)_value_size];
    for (int i = 0; i < _value_size; ++i) {
        s[i] = value_alphanum[rand() % (sizeof(value_alphanum) - 1)];
    }
    s[_value_size] = '\0';
    return s;
}

std::vector<std::string> StringSplit(const std::string& arg, char delim) {
    std::vector<std::string> splits;
    std::stringstream ss(arg);
    std::string item;
    while (std::getline(ss, item, delim)) {
        splits.push_back(item);
    }
    return splits;
}

void generate_workload() {

    //std::cout << "Generating workload ..." << std::endl;
    long total_operation_count = insert_count + update_count + point_delete_count + range_delete_count + point_query_count + range_query_count;
    std::cout << "Total operation count = " << total_operation_count << std::endl << std::flush;
    std::set<Key> tmp_insert_pool_set; 
    std::vector<Key> tmp_insert_pool_vec; 
    if (load_from_existing_workload) {
        std::ifstream fin(file_path+FILENAME);
        // std::cout << "WL_GEN :: preload input file = " << file_path << FILENAME << std::endl;
        //load from existing workload file: NOTE: ONLY CONSIDERS INSERTS!!!
        if (fin.good()) {
            std::vector<std::string> splits;   
            std::string line;
            while (getline(fin, line)) {
                splits = StringSplit(line, ' '); 
                Key key = splits[1];
                if(!STRING_KEY_ENABLED){
                   key = atoi(splits[1].c_str());
                }
                if(tmp_insert_pool_set.find(key) == tmp_insert_pool_set.end()){
                    tmp_insert_pool_set.insert(key);
                    global_insert_pool.push_back(key);
                    insert_pool.push_back(key);
                }
            }

        sorted = false;
        }
        fin.close();
    }
    if (insert_count + insert_pool.size() < point_delete_count + range_delete_count * range_delete_selectivity * insert_count) {
        std::cout << "\033[1;31m ERROR:\033[0m insert_count < point_delete_count + range_delete_count * range_delete_selectivity * insert_count" << std::endl;
        exit(0);
    }
    std::ofstream fp;
    if(out_filename.compare("") == 0){

        fp.open(file_path+FILENAME);
    //   std::cout << "WL_GEN :: output file = " << file_path << FILENAME << std::endl;
    }
    else {
        fp.open(out_filename);
    //   std::cout << "WL_GEN :: output file = " << file_path << out_filename << std::endl;
    }

    long _insert_count = 0;
    long _update_count = 0;
    long _point_delete_count = 0;
    long _range_delete_count = 0;
    long _point_query_count = 0;
    long _non_existing_point_query_count = 0;
    long _existing_point_query_count = 0;
    long _range_query_count = 0;
    long _total_operation_count = 0;
    long _effective_ingestion_count = 0; // insert = +1 ; update = 0 ; point_delete = -1 ; range_delete = -x
    int choice_domain = 6;
    int flag = 0;

    uint32_t num_char = (std::string(Key::key_alphanum)).size();
    uint32_t num_preserved_bits = 10;
    // generate unique key-value pairs in advance
    
    if(STRING_KEY_ENABLED){
        insertIndexGenerator = new Generator(insert_dist, 0, num_char*num_char-1, insert_norm_mean_percentile*num_char*num_char, insert_norm_stddev*num_char, insert_beta_alpha, insert_beta_beta, insert_zipf_alpha, num_char*num_char);
    }else{
        uint32_t int32_preserved_insert_domain_size = pow(2, num_preserved_bits);
        insertIndexGenerator = new Generator(insert_dist, 0, int32_preserved_insert_domain_size-1, insert_norm_mean_percentile*int32_preserved_insert_domain_size, insert_norm_stddev*int32_preserved_insert_domain_size, insert_beta_alpha, insert_beta_beta, insert_zipf_alpha, int32_preserved_insert_domain_size);
    }
    char prefix[] = "00";
    
    while(_insert_count < insert_count){
	Key key;	
        Key key_suffix;
        // std::cout << key << std::endl;
        do{
            
            uint32_t index = insertIndexGenerator->getNext();
            if(STRING_KEY_ENABLED){
	        key_suffix = Key::get_key(key_size - 2, STRING_KEY_ENABLED);
	        prefix[0] = Key::key_alphanum[(index/62)%62];
	        prefix[1] = Key::key_alphanum[index%62];
	        key = Key(prefix);
	        key = key + key_suffix;
            }else{
	        key_suffix = Key::get_key(32 - num_preserved_bits, STRING_KEY_ENABLED);
                index <<= (32 - num_preserved_bits);
                key = Key(key_suffix.key_int32_ | index);
            }
	}while(tmp_insert_pool_set.find(key) != tmp_insert_pool_set.end());
        tmp_insert_pool_set.insert(key);
        global_insert_pool.push_back(key);
	_insert_count++;
//=======
//    if(STRING_KEY_ENABLED) {
//        insertIndexGenerator = new Generator(insert_dist, 0, num_char*num_char-1, insert_norm_mean_percentile*num_char*num_char, insert_norm_stddev*num_char, insert_beta_alpha, insert_beta_beta, insert_zipf_alpha, num_char*num_char);
//    }
//    else {
//        uint32_t int32_preserved_insert_domain_size = pow(2, num_preserved_bits);
//        insertIndexGenerator = new Generator(insert_dist, 0, int32_preserved_insert_domain_size-1, insert_norm_mean_percentile*int32_preserved_insert_domain_size, insert_norm_stddev*int32_preserved_insert_domain_size, insert_beta_alpha, insert_beta_beta, insert_zipf_alpha, int32_preserved_insert_domain_size);
//    }
//    char prefix[] = "00";
//    
//    while(_insert_count < insert_count) {
//        Key key;	
//        Key key_suffix;
//        // std::cout << key << std::endl;
//        do {
//            uint32_t index = insertIndexGenerator->getNext();
//            if (STRING_KEY_ENABLED) {
//                key_suffix = Key::get_key(key_size - 2, STRING_KEY_ENABLED);
//                prefix[0] = Key::key_alphanum[(index/62)%62];
//                prefix[1] = Key::key_alphanum[index%62];
//                key = Key(prefix);
//                key = key + key_suffix;
//            }
//            else {
//                key_suffix = Key::get_key(32 - num_preserved_bits, STRING_KEY_ENABLED);
//                index <<= (32 - num_preserved_bits);
//                key = Key(key_suffix.key_int32_ | index);
//            }
//        } while(tmp_insert_pool_set.find(key) != tmp_insert_pool_set.end());
//        tmp_insert_pool_set.insert(key);
//        global_insert_pool.push_back(key);
//        _insert_count++;
//>>>>>>> 084785aa2e580ba6a054ed624ad772a6ba060ff9
    }
    _insert_count = 0;
    sort(tmp_insert_pool_vec.begin(), tmp_insert_pool_vec.end()); 

    // generate non-existing keys in advance
    long _maximum_unique_non_existing_point_query_count = 0;
    while (_maximum_unique_non_existing_point_query_count < maximum_unique_non_existing_point_query_count) {
        Key key;
        if (STRING_KEY_ENABLED)
            key = Key::get_key(key_size, STRING_KEY_ENABLED);
        else
            key = Key::get_key(32, STRING_KEY_ENABLED);
        while(tmp_insert_pool_set.find(key) != tmp_insert_pool_set.end() || global_non_existing_key_set.find(key) != global_non_existing_key_set.end()){
            if (STRING_KEY_ENABLED)
                key = Key::get_key(key_size, STRING_KEY_ENABLED);
            else
                key = Key::get_key(32, STRING_KEY_ENABLED);
        }
        global_non_existing_key_set.insert(key);
        global_non_existing_key_pool.push_back(key);
        _maximum_unique_non_existing_point_query_count++;
    }
    tmp_insert_pool_set.clear();
    sort(global_non_existing_key_pool.begin(), global_non_existing_key_pool.end()); 
    double scaling_ratio = 1.0;
    if(STRING_KEY_ENABLED) scaling_ratio = num_char;
    nonExistingPointLookupIndexGenerator = new Generator(non_existing_point_lookup_dist, 0, global_non_existing_key_pool.size() - 1, non_existing_point_lookup_norm_mean_percentile*global_non_existing_key_pool.size(), non_existing_point_lookup_norm_stddev*global_non_existing_key_pool.size()/scaling_ratio, non_existing_point_lookup_beta_alpha, non_existing_point_lookup_beta_beta, non_existing_point_lookup_zipf_alpha, global_non_existing_key_pool.size());

     std::vector<int> update_global_index_mapping;
     if (update_count > 0) {
         updateIndexGenerator = new Generator(update_dist, 0, global_insert_pool.size() - 1, update_norm_mean_percentile*global_insert_pool.size(), update_norm_stddev*global_insert_pool.size()/scaling_ratio, update_beta_alpha, update_beta_beta, update_zipf_alpha, global_insert_pool.size(), update_global_index_mapping);
     }

    while (_total_operation_count < total_operation_count) {
        int choice = get_choice(insert_pool.size(), insert_count, update_count, point_delete_count, range_delete_count, point_query_count, range_query_count, _insert_count, _update_count, _point_delete_count, _range_delete_count, _point_query_count, _range_query_count);
        // std::cout << "choice = " << choice << std::endl;

        if (choice == 0)
            continue;

        else if (choice == 1) { // INSERT
            long global_insert_pool_size = global_insert_pool.size();
            long index = (long)(rand() % (global_insert_pool_size - _insert_count));
            Key key = global_insert_pool[index+_insert_count];
            //swap the key with the first element after inserted ones
            global_insert_pool[index+_insert_count] = global_insert_pool[_insert_count];
            global_insert_pool[_insert_count] = key;
            // std::cout << value << std::endl;

            Key value = get_value(entry_size - key_size);
            if(sorted) {
                std::vector<Key>::iterator it = std::upper_bound(insert_pool.begin(), insert_pool.end(), key);
                insert_pool.insert(it, key);
            }
            else {
                insert_pool.push_back(key);
            }
            global_insert_pool_set.insert(key);
            // std::cout << "I " << key << " " << value << std::endl;
            fp << "I " << key << " " << value << std::endl;
            _insert_count++;
            _effective_ingestion_count++;
            _total_operation_count++;
        }

        else if (choice == 2) { // UPDATE
            std::vector<int> index_mapping;
            if(!sorted){
                if(existing_point_lookup_dist == 1){
		    std::cout << "sort here" << std::endl;
                    sort(insert_pool.begin(), insert_pool.end());
                    double scaling_ratio = 1.0;
                    sorted = true;
                
                    if(STRING_KEY_ENABLED) 
                        scaling_ratio = num_char;

                    if(updateIndexGenerator != nullptr){
			std::cout << "renew update generator" << std::endl;
                    	index_mapping = updateIndexGenerator->index_mapping;
                    	delete updateIndexGenerator;
         	    	updateIndexGenerator = new Generator(update_dist, 0, global_insert_pool.size() - 1, update_norm_mean_percentile*global_insert_pool.size(), update_norm_stddev*global_insert_pool.size()/scaling_ratio, update_beta_alpha, update_beta_beta, update_zipf_alpha, global_insert_pool.size(), update_global_index_mapping);
                     }
                }
                
               	 
                
            }


            long index = updateIndexGenerator->getNext();
	    if (index >= insert_pool.size()) { // Generate an insert instead here
		Key key = global_insert_pool[index];
            	global_insert_pool[index] = global_insert_pool[_insert_count];
            	global_insert_pool[_insert_count] = key;
                Key value = get_value(entry_size - key_size);
		if(sorted) {
                	std::vector<Key>::iterator it = std::upper_bound(insert_pool.begin(), insert_pool.end(), key);
                	insert_pool.insert(it, key);
            	} else {
                	insert_pool.push_back(key);
            	}
		global_insert_pool_set.insert(key);
                fp << "I " << key << " " << value << std::endl;
                _insert_count++;
                _effective_ingestion_count++;
	    } else {
		Key key = insert_pool[index];
            	// std::cout << key << std::endl;
            	Key value = get_value(entry_size - key_size);
            	// std::cout << value << std::endl;
            	// std::cout << "U " << key << " " << value << std::endl;
            	fp << "U " << key << " " << value << std::endl;
            	_update_count++;
	    }
            
            _total_operation_count++;
        }

        else if (choice == 3) { // POINT DELETE
            // the following if block ensures that all updates are completed before a database is emptied ( in cases where insert_count == point_delete_count)
            if (insert_count == point_delete_count && _insert_count == insert_count && _point_delete_count == point_delete_count - 1 && _update_count < update_count ) {
                std::cout << "pausing delete to facilitate all remaining updates ... " << std::endl;
            }
            else {
                // std::cout << "_insert_count " << _insert_count << " ; _update_count " << _update_count << std::endl;
                long insert_pool_size = insert_pool.size();
                long index = (long)(rand() % insert_pool_size);
                Key key = insert_pool[index];
                // std::cout << key << std::endl;
                global_insert_pool_set.erase(key);
                insert_pool.erase(insert_pool.begin() + index);
	        std::vector<int> index_mapping;
                if(updateIndexGenerator != nullptr && update_count > 0){
		    index_mapping = updateIndexGenerator->index_mapping;
                    delete updateIndexGenerator;
         	    updateIndexGenerator = new Generator(update_dist, 0, global_insert_pool.size() - 1, update_norm_mean_percentile*global_insert_pool.size(), update_norm_stddev*global_insert_pool.size()/scaling_ratio, update_beta_alpha, update_beta_beta, update_zipf_alpha, global_insert_pool.size(), update_global_index_mapping);
                }
		index_mapping.clear();
                if(existingPointLookupIndexGenerator != nullptr && point_query_count > 0){
		    index_mapping = existingPointLookupIndexGenerator->index_mapping;

                    delete existingPointLookupIndexGenerator;
                    existingPointLookupIndexGenerator = new Generator(existing_point_lookup_dist, 0, insert_pool.size() - 1, existing_point_lookup_norm_mean_percentile*insert_pool.size(), existing_point_lookup_norm_stddev*insert_pool.size()/scaling_ratio, existing_point_lookup_beta_alpha, existing_point_lookup_beta_beta, existing_point_lookup_zipf_alpha, insert_pool.size(), index_mapping);
                }

                // std::cout << "D " << key << std::endl;
                fp << "D " << key << " " << std::endl;
                _point_delete_count++;
                _effective_ingestion_count--;
                _total_operation_count++;
            }
            
        }

        else if (choice == 4) { // RANGE DELETE
            // selectivity is computed on the current size of the insert pool (insert_pool.size()) and NOT the total inserts to be made (insert_count)

            // the following code-block generates range selectivity as a random number 

            // for now we use the hardcoded range selectivity 
            long insert_pool_size = insert_pool.size();
            long entries_in_range_delete = -1;
            if ( (float)range_delete_selectivity * insert_pool_size > 0 && (float)range_delete_selectivity * insert_pool_size < 1 ) // computed on the current size of insert pool 
                entries_in_range_delete = 1;
            else 
                entries_in_range_delete = floor( (float)range_delete_selectivity * insert_pool_size ); // computed on the current size of insert pool 
            long start_index = (long)(rand() % insert_pool_size);
            long end_index = -1;
            if (start_index + entries_in_range_delete > insert_pool_size ) {
                // std::cout << "start index (= " << start_index << ") + entries_in_range_delete (= " << entries_in_range_delete << ") > insert_pool_size (= " << insert_pool_size << ")" << std::endl;
                start_index -= ( start_index + entries_in_range_delete - insert_pool_size );
                // std::cout << "start index (= " << start_index << ") + entries_in_range_delete (= " << entries_in_range_delete << ") > insert_pool_size (= " << insert_pool_size << ")" << std::endl;
            }
            // std::cout << "ELSE: start index (= " << start_index << ") + entries_in_range_delete (= " << entries_in_range_delete << ") > insert_pool_size (= " << insert_pool_size << ")" << std::endl;
            end_index = start_index + entries_in_range_delete - 1;
            if (start_index < 0 || entries_in_range_delete == 0) {
                std::cout << "not enough entries in tree for range delete -- skipping ... ; insert_pool_size = " << insert_pool_size << std::endl;
                std::cout << "start_index = " << start_index << " ; entries_in_range_delete = " << entries_in_range_delete << std::endl;
                flag++;
                if (flag > 20)
                    exit(-1);
            }
            else {
                //std::cout << "Issuing range delete from index " << start_index << " to " << end_index << std::endl;

                // std::cout << "Before sorting = ";
                // for (int i = 0; i < insert_pool.size(); ++i)
                //     std::cout << insert_pool[i] << ' ';
                // std::cout << std::endl;

                sort(insert_pool.begin(), insert_pool.end()); 
                Key start_key = insert_pool[start_index];
                Key end_key = insert_pool[end_index];

                // std::cout << "After sorting and before range deleting = ";
                // for (int i = 0; i < insert_pool.size(); ++i)
                //     std::cout << insert_pool[i] << ' ';
                // std::cout << std::endl;

                //std::cout << "Deleting entries from index " << start_index << " to " << end_index << std::endl;
		for(int i = start_index; i < end_index + 1; i++){
            global_insert_pool_set.erase(insert_pool[start_index]);
		}
                insert_pool.erase(insert_pool.begin() + start_index, insert_pool.begin() + end_index + 1 );

                // std::cout << "After range deleting = ";
                // for (int i = 0; i < insert_pool.size(); ++i)
                //     std::cout << insert_pool[i] << ' ';
                // std::cout << std::endl;

                //std::cout << "R " << start_key << " " << end_key << std::endl;
                fp << "R " << start_key << " " << end_key << std::endl;
                _range_delete_count++;
                _effective_ingestion_count -= entries_in_range_delete;
                _total_operation_count++;
            }
            
        }

        else if (choice == 5) { // POINT QUERY
            // the following if block ensures that all point queries are completed before a database is emptied ( in cases where insert_count == point_delete_count)
            if (insert_count == point_delete_count && _insert_count == insert_count && _point_delete_count == point_delete_count - 1 && _point_query_count < point_query_count ) {
                std::cout << "pausing delete to facilitate all remaining point queries ... " << std::endl;
            }
            else {
		float query_type = (float) rand()/RAND_MAX;
		if(query_type <= zero_result_point_lookup_proportion && _non_existing_point_query_count < non_existing_point_query_count) {
                    Key key = global_non_existing_key_pool[nonExistingPointLookupIndexGenerator->getNext()];
                    //std::cout << "Q' " << key << "\t" << _non_existing_point_query_count << " < " << non_existing_point_query_count << std::endl;
		    fp << "Q " << key << std::endl;
                    _point_query_count++;
	            _non_existing_point_query_count++;
		    _total_operation_count++;
		    


		}
		else if(_existing_point_query_count < existing_point_query_count){
		// std::cout << "_insert_count " << _insert_count << " ; _point_query_count " << _point_query_count << std::endl;
		std::vector<int> index_mapping;
		if(!sorted){
                        if(existing_point_lookup_dist == 1){
                            sort(insert_pool.begin(), insert_pool.end());
                            double scaling_ratio = 1.0;
                            if(STRING_KEY_ENABLED) 
                                scaling_ratio = num_char;

                        }
                        if(existing_point_lookup_dist != 0 && existingPointLookupIndexGenerator != nullptr){

			       index_mapping = existingPointLookupIndexGenerator->index_mapping;
                               delete existingPointLookupIndexGenerator;
                               existingPointLookupIndexGenerator = nullptr;
			   sorted = true;
                      }
    			
	        }
                if(existingPointLookupIndexGenerator == nullptr){
                	existingPointLookupIndexGenerator = new Generator(existing_point_lookup_dist, 0, insert_pool.size() - 1, existing_point_lookup_norm_mean_percentile*insert_pool.size(), existing_point_lookup_norm_stddev*insert_pool.size()/scaling_ratio, existing_point_lookup_beta_alpha, existing_point_lookup_beta_beta, existing_point_lookup_zipf_alpha, insert_pool.size(), index_mapping);
                }
                long index = 0;
                if(existing_point_lookup_dist == 0){
                    index = rand()%insert_pool.size() ;
                }else{
                    index = (long)(existingPointLookupIndexGenerator->getNext());
                }
                Key key = insert_pool[index];
                // std::cout << key << std::endl;

                //std::cout << "Q " << key << "\t" << _existing_point_query_count << " < " << existing_point_query_count << std::endl;
                //std::cout << "Q " << key  << std::endl;
                fp << "Q " << key << std::endl;
                _point_query_count++;
		_existing_point_query_count++;
                _total_operation_count++;
//=======
//		else if (_existing_point_query_count < existing_point_query_count) {
//            // std::cout << "_insert_count " << _insert_count << " ; _point_query_count " << _point_query_count << std::endl;
//            std::vector<int> index_mapping;
//            if (!sorted) {
//                sort(insert_pool.begin(), insert_pool.end());
//                double scaling_ratio = 1.0;
//                if (STRING_KEY_ENABLED) scaling_ratio = num_char;
//                sorted = true;
//                if (existingPointLookupIndexGenerator != nullptr) {
//                    index_mapping = existingPointLookupIndexGenerator->index_mapping;
//                    delete existingPointLookupIndexGenerator;
//                    existingPointLookupIndexGenerator = nullptr;
//                }
//            }
//                    if (existingPointLookupIndexGenerator == nullptr) {
//                        existingPointLookupIndexGenerator = new Generator(existing_point_lookup_dist, 0, insert_pool.size() - 1, existing_point_lookup_norm_mean_percentile*insert_pool.size(), existing_point_lookup_norm_stddev*insert_pool.size()/scaling_ratio, existing_point_lookup_beta_alpha, existing_point_lookup_beta_beta, existing_point_lookup_zipf_alpha, insert_pool.size(), index_mapping);
//                    }
//                    long index = (long)(existingPointLookupIndexGenerator->getNext());
//                    Key key = insert_pool[index];
//                    // std::cout << key << std::endl;
//
//                    //std::cout << "Q " << key << "\t" << _existing_point_query_count << " < " << existing_point_query_count << std::endl;
//                    //std::cout << "Q " << key  << std::endl;
//                    fp << "Q " << key << std::endl;
//                    _point_query_count++;
//                    _existing_point_query_count++;
//                    _total_operation_count++;
//>>>>>>> 084785aa2e580ba6a054ed624ad772a6ba060ff9

		}
                
        }
            
        }

        else if (choice == 6) { // RANGE QUERY
            // selectivity is computed on the current size of the insert pool (insert_pool.size()) and NOT the total inserts to be made (insert_count)

            // the following code-block generates range selectivity as a random number 

            // for now we use the hardcoded range selectivity
            long insert_pool_size = insert_pool.size();
            long entries_in_range_query = floor(range_query_selectivity * insert_pool_size); // computed on the current size of insert pool  
            long start_index = (long)(rand() % insert_pool_size);
            long end_index = -1;
            if (start_index + entries_in_range_query > insert_pool_size ) {
                // std::cout << "start index (= " << start_index << ") + entries_in_range_query (= " << entries_in_range_query << ") > insert_pool_size (= " << insert_pool_size << ")" << std::endl;
                start_index -= ( start_index + entries_in_range_query - insert_pool_size );
                // std::cout << "start index (= " << start_index << ") + entries_in_range_query (= " << entries_in_range_query << ") > insert_pool_size (= " << insert_pool_size << ")" << std::endl;
            }
            // std::cout << "ELSE: start index (= " << start_index << ") + entries_in_range_query (= " << entries_in_range_query << ") > insert_pool_size (= " << insert_pool_size << ")" << std::endl;
            end_index = start_index + entries_in_range_query - 1;
            if (start_index < 0 || entries_in_range_query == 0) {
                std::cout << "not enough entries in tree for range query -- skipping ... ; insert_pool_size = " << insert_pool_size << std::endl;
                std::cout << "start_index = " << start_index << " ; entries_in_range_query = " << entries_in_range_query << std::endl;
                flag++;
                if (flag > 20)
                    ;//exit(-1);
            }
            else {
                //std::cout << "Issuing range query from index " << start_index << " to " << end_index << std::endl;

                // std::cout << "Before sorting = ";
                // for (int i = 0; i < insert_pool.size(); ++i)
                //     std::cout << insert_pool[i] << ' ';
                // std::cout << std::endl;
                if(!sorted){
                    sort(insert_pool.begin(), insert_pool.end()); 
	                //sorted = true; //if the number of range queries increases by a lot, this might be a problem in terms of execution speed!!!
                }
                Key start_key = insert_pool[start_index];
                Key end_key = insert_pool[end_index];

                // std::cout << "After sorting and before range deleting = ";
                // for (int i = 0; i < insert_pool.size(); ++i)
                //     std::cout << insert_pool[i] << ' ';
                // std::cout << std::endl;

                //std::cout << "S " << start_key << " " << end_key << std::endl;
                fp << "S " << start_key << " " << end_key << std::endl;
                _range_query_count++;
                _total_operation_count++;
            }
            
        }
        
        // Progress bar
        //  if (total_operation_count > 100)
        //      if(_total_operation_count % (total_operation_count/100) == 0) 
        //          showProgress(total_operation_count, _total_operation_count);

    }

    print_workload_parameters(_insert_count, _update_count, _point_delete_count, _range_delete_count, _effective_ingestion_count);
}

void print_workload_parameters(int _insert_count, int _update_count, int _point_delete_count,int  _range_delete_count,int _effective_ingestion_count){
    std::cout <<"Workload_parameters: "
    << "entry_size = "<< entry_size << ", " 
    << "key_size = "<< key_size << ", " 
    << "lambda = "<< lambda << ", " 
    << "insert_count = "<< _insert_count << ", " 
    << "update_count = " << _update_count << ", " 
    << "point_delete_count = " << _point_delete_count << ", " 
    << "range_delete_count = "<< _range_delete_count << ", " 
    << "range_delete_selectivity = "<< range_delete_selectivity << ", "
    << "zero_result_point_delete_proportion = "<< zero_result_point_delete_proportion << ", " 
    << "effective_ingestion_count = "<< _effective_ingestion_count << ", " 
    << "point_query_count = "<< point_query_count << ", " 
    << "range_query_count = "<< range_query_count << ", " 
    << "range_query_selectivity = "<< range_query_selectivity << ", " 
    << "zero_result_point_lookup_proportion= "<< zero_result_point_lookup_proportion << ", " 
    << "existing_point_query_count = "<< existing_point_query_count << ", " 
    << "non_existing_point_query_count = "<< non_existing_point_query_count << ", " 
    << "maximum_unique_non_existing_point_query_count = "<< maximum_unique_non_existing_point_query_count << ", " 
    << "maximum_unique_existing_point_query_count = "<< maximum_unique_existing_point_query_count << ", " 
    << "insert_dist = "<< insert_dist << ", " 
    << "insert_norm_mean_percentile = "<< insert_norm_mean_percentile << ", " 
    << "insert_norm_stddev = "<< insert_norm_stddev << ", " 
    << "insert_beta_alpha = "<< insert_beta_alpha << ", " 
    << "insert_beta_beta = "<< insert_beta_beta << ", " 
    << "insert_zipf_alpha = "<< insert_zipf_alpha << ", " 
    << "update_dist = "<< update_dist << ", " 
    << "update_norm_mean_percentile = "<< update_norm_mean_percentile << ", " 
    << "update_norm_stddev = "<< update_norm_stddev << ", " 
    << "update_beta_alpha = "<< update_beta_alpha << ", " 
    << "update_beta_beta = "<< update_beta_beta << ", " 
    << "update_zipf_alpha = "<< update_zipf_alpha << ", " 
    << "non_existing_point_lookup_dist = "<< non_existing_point_lookup_dist << ", " 
    << "non_existing_point_lookup_norm_mean_percentile = "<< non_existing_point_lookup_norm_mean_percentile << ", " 
    << "non_existing_point_lookup_norm_stddev = "<< non_existing_point_lookup_norm_stddev << ", " 
    << "non_existing_point_lookup_beta_alpha = "<< non_existing_point_lookup_beta_alpha << ", " 
    << "non_existing_point_lookup_beta_beta = "<< non_existing_point_lookup_beta_beta << ", " 
    << "non_existing_point_lookup_zipf_alpha = "<< non_existing_point_lookup_zipf_alpha << ", " 
    << "existing_point_lookup_dist = "<< existing_point_lookup_dist << ", " 
    << "existing_point_lookup_norm_mean_percentile = "<< existing_point_lookup_norm_mean_percentile << ", " 
    << "existing_point_lookup_norm_stddev = "<< existing_point_lookup_norm_stddev << ", " 
    << "existing_point_lookup_beta_alpha = "<< existing_point_lookup_beta_alpha << ", " 
    << "existing_point_lookup_beta_beta = "<< existing_point_lookup_beta_beta << ", " 
    << "existing_point_lookup_zipf_alpha = "<< existing_point_lookup_zipf_alpha << ", "
    << "sorted = "<< sorted << ", " 
    << "num_insert_key_prefix = "<< num_insert_key_prefix
    <<std::endl;
}



int get_choice(long insert_pool_size, long insert_count, long update_count, long point_delete_count, long range_delete_count, long point_query_count, long range_query_count, long _insert_count, long _update_count, long _point_delete_count, long _range_delete_count, long _point_query_count, long _range_query_count) {
    long total_operation_count = (insert_count - _insert_count) + (update_count - _update_count) + (point_delete_count - _point_delete_count) + (range_delete_count - _range_delete_count) + (point_query_count - _point_query_count) + (range_query_count - _range_query_count);
    if(total_operation_count == 0) return 0;
    float insert_fraction = (float) (insert_count - _insert_count) / total_operation_count;
    float update_fraction = (float) (update_count - _update_count) / total_operation_count;
    float point_delete_fraction = (float) (point_delete_count - _point_delete_count) / total_operation_count;
    float range_delete_fraction = (float) (range_delete_count - _range_delete_count) / total_operation_count;
    float point_query_fraction = (float) (point_query_count - _point_query_count) / total_operation_count;
    float range_query_fraction = (float) (range_query_count - _range_query_count) / total_operation_count;
    // float cumulative_fraction = insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction + point_query_fraction + range_query_fraction;
    int choice_domain = 6;
    int choice = 0;

    float rand_float = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
    // std::cout << cumulative_fraction << " " << rand_float << std::endl;

    if (rand_float < insert_fraction) choice = 1;
    else if (rand_float < insert_fraction + update_fraction) choice = 2;
    else if (rand_float < insert_fraction + update_fraction + point_delete_fraction) choice = 3;
    else if (rand_float < insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction) choice = 4;
    else if (rand_float < insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction + point_query_fraction) choice = 5;
    else if (rand_float <= insert_fraction + update_fraction + point_delete_fraction + range_delete_fraction + point_query_fraction + range_query_fraction) choice = 6;

    // std::cout << "choice = " << choice << std::endl;
    switch (choice) {
        case 6: 
            if (_range_query_count < range_query_count && insert_pool_size > 0 && _insert_count >= RQ_THRESHOLD * insert_count) 
                break;
            choice--;
        case 5: 
            if (_point_query_count < point_query_count && insert_pool_size > 0 && _insert_count >= PQ_THRESHOLD * insert_count) 
                break;
            // choice = (choice + 1)%choice_domain;
            choice--;
        case 4: 
            if (_range_delete_count < range_delete_count && insert_pool_size > 0 && _insert_count >= RD_THRESHOLD * insert_count) 
                break;
            choice--;
        case 3: 
            if (_point_delete_count < point_delete_count && insert_pool_size > 0 && _insert_count >= PD_THRESHOLD * insert_count) 
                break;
            // choice = (choice + 1)%choice_domain;
            choice--;
        case 2: 
            if (_update_count < update_count && insert_pool_size > 0 && _insert_count >= U_THRESHOLD * insert_count) 
                break;
            // choice = (choice + 1)%choice_domain;
            choice--;
        case 1: // for inserts
            if (_insert_count < insert_count) 
                break;
            // choice = (choice + 1)%choice_domain;
            choice = 0;
    }

    return choice;
}

int main(int argc, char *argv[]) {
    /*{
        std::cout << "Size of int = " << sizeof(int) << std::endl;
        std::cout << "Size of long = " << sizeof(long) << std::endl;
        std::cout << "Size of char = " << sizeof(char) << std::endl;
        std::cout << "Size of \"1234567890\" = " << sizeof("1234567890") << std::endl;
        int a[0] = {};
        std::cout << "Size of int a[0] = " << sizeof(a) << std::endl;
        int *b;
        std::cout << "Size of int *b = " << sizeof(b) << std::endl;
        long c[0] = {};
        std::cout << "Size of long a[0] = " << sizeof(c) << std::endl;
        long *d;
        std::cout << "Size of long *d = " << sizeof(d) << std::endl;
        char *e;
        std::cout << "Size of long *d = " << sizeof(d) << std::endl;
        e = new char[2];
        e[0] = 's'; e[1] = 'e';
        std::cout << "Size of long *e after instering two elements = " << sizeof(e) << std::endl;

        std::vector<long> vint;
        vint.push_back(6);
        std::cout << "Size of vint = " << sizeof(vint) << std::endl;
        std::cout << "Size of vint.size() = " << vint.size() << std::endl;
        std::cout << "Size of vint.max_size() = " << vint.max_size() << std::endl;
        std::cout << "Size of vint.capacity() = " << vint.capacity() << std::endl;
        std::cout << "Size of vint.empty() = " << vint.empty() << std::endl;
    }*/
    //std::srand((unsigned int)std::time(NULL));

    if (parse_arguments2(argc, argv)){
        exit(1);
    }


    if(!entry_size) {
        std::cout << "\033[1;31m ERROR:\033[0m entry_size = 0" << std::endl;
        exit(0);
    }

    
    /* 
>>>>>>> 084785aa2e580ba6a054ed624ad772a6ba060ff9
    if (log(insert_count*num_insert_key_prefix)/log(62) > lambda*entry_size - 2) {
        std::cout << "\033[1;31m ERROR:\033[0m too small key size to support sufficient unique inserts" << std::endl;
        exit(0);
    }*/

    generate_workload();

    /*
    if (lambda == -1) { // this means, the size of the key is equal to the size of uint32_t, i.e., 4 bytes
        generate_workload_with_key_as_uint32_t();
    }
    
    else { // this means, we want to vary the key length. So we change the datatype of key from uint32_t to string
        generate_workload_with_key_as_string();
        
    }*/

    // std::cout << "End of main() ..." << std::endl;

    // if(!STRING_KEY_ENABLED && (lambda > 0 && lambda < 1)){
    //     std::cerr << "\033[0;33m Warning:\033[0m STRING_ENABLED is false, lambda is invalid in this case and key_size is set as " << sizeof(uint32_t) << "Bytes by default." << std::endl;
    // }

    // if(load_from_existing_workload){
    //     std::cerr << "\033[0;33m Warning:\033[0m Remember to keep key size and key type consistent when you enable preloading." << std::endl;
    // }

    // if(load_from_existing_workload && out_filename.compare("") == 0){
    //     std::cerr << "\033[0;33m Warning:\033[0m workload.txt will be overwritten ! " << std::endl;
    // }
    return 1;
}



int parse_arguments2(int argc, char *argv[]) {
  args::ArgumentParser parser("workload_gen_parser", "");

  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::DontCare);
/*
  args::Group group1(parser, "This group is all exclusive:", args::Group::Validators::AtMostOne);
  args::Group group2(parser, "Path is needed:", args::Group::Validators::All);
  args::Group group3(parser, "This group is all exclusive (either N or L):", args::Group::Validators::Xor);
  args::Group group4(parser, "Optional switches and parameters:", args::Group::Validators::DontCare);
  args::Group group5(parser, "Optional less frequent switches and parameters:", args::Group::Validators::DontCare);
*/

  args::ValueFlag<long> insert_cmd(group1, "I", "Number of inserts [def: 0]", {'I', "insert"});
  args::ValueFlag<long> update_cmd(group1, "U", "Number of updates [def: 0]", {'U', "update"});
  args::ValueFlag<long> point_delete_cmd(group1, "D", "Number of point deletes [def: 0]", {'D', "point_delete"});
  args::ValueFlag<long> range_delete_cmd(group1, "R", "Number of range deletes [def: 0]", {'R', "range_delete"});
  args::ValueFlag<float> range_delete_selectivity_cmd(group1, "y", "Range delete selectivity [def: 0]", {'y', "range_delete_selectivity"});
  args::ValueFlag<long> point_query_cmd(group1, "Q", "Number of point queries [def: 0]", {'Q', "point_query"});
  args::ValueFlag<long> range_query_cmd(group1, "S", "Number of range queries [def: 0]", {'S', "range_query"});
  args::ValueFlag<float> range_query_selectivity_cmd(group1, "Y", "Range query selectivity [def: 0]", {'Y', "range_query_selectivity"});
  args::ValueFlag<float> zero_result_point_delete_proportion_cmd(group1, "z", "Proportion of zero-result point deletes [def: 0]", {'z', "zero_result_point_delete_proportion"});
  args::ValueFlag<float> zero_result_point_lookup_proportion_cmd(group1, "Z", "Proportion of zero-result point lookups [def: 0]", {'Z', "zero_result_point_lookup_proportion"});
  args::ValueFlag<float> unique_zero_result_point_lookup_proportion_cmd(group1, "UZ", "Proportion of maximum unique zero-result point lookups [def: 0.5]", {"UZ", "unique_zero_result_point_lookup_proportion"});
  args::ValueFlag<float> maximum_unique_existing_point_lookup_proportion_cmd(group1, "UE", "Proportion of maximum unique exising point lookups [def: 0.5]", {"UE", "maximum_unique_existing_point_lookup_proportion"});

  args::ValueFlag<uint32_t> entry_size_cmd(group1, "E", "Entry size (in bytes) [def: 8]", {'E', "entry_size"});
  args::ValueFlag<float> lambda_cmd(group1, "L", "lambda = key_size / (key_size + value_size) [def: 0.5]", {'L', "lambda"});

  args::Flag load_from_existing_workload_cmd(group1, "Preload", "preload from workload.txt", {"PL", "preloading"});
  args::ValueFlag<std::string> out_filename_cmd(group1, "OP", "output path [def: 0]", {"OP", "output-path"});
  // distribution params
  args::ValueFlag<uint32_t> insert_dist_cmd(group1, "ID", "Insert Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"ID", "insert_distribution"});
  args::ValueFlag<float> insert_dist_norm_mean_percentile_cmd(group1, "ID_Norm_Mean_Percentile", ", def: 0.5]", {"ID_NMP", "insert_distribution_norm_mean_percentile"});
  args::ValueFlag<float> insert_dist_norm_stddev_cmd(group1, "ID_Norm_Stddev", ", def: 1]", {"ID_NDEV", "insert_distribution_norm_standard_deviation"});
  args::ValueFlag<float> insert_dist_beta_alpha_cmd(group1, "ID_Beta_Alpha", ", def: 1.0]", {"ID_BALPHA", "insert_distribution_beta_alpha"});
  args::ValueFlag<float> insert_dist_beta_beta_cmd(group1, "ID_Beta_Beta", ", def: 1.0]", {"ID_BBETA", "insert_distribution_beta_beta"});
  args::ValueFlag<float> insert_dist_zipf_alpha_cmd(group1, "ID_Zipf_Alpha", ", def: 1.0]", {"ID_ZALPHA", "insert_distribution_zipf_alpha"});

  args::ValueFlag<uint32_t> update_dist_cmd(group1, "UD", "Update Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"UD", "update_distribution"});
  args::ValueFlag<float> update_dist_norm_mean_percentile_cmd(group1, "UD_Norm_Mean_Percentile", ", def: 0.5]", {"UD_NMP", "update_distribution_norm_mean_percentile"});
  args::ValueFlag<float> update_dist_norm_stddev_cmd(group1, "UD_Norm_Stddev", ", def: 1]", {"UD_NDEV", "update_distribution_norm_standard_deviation"});
  args::ValueFlag<float> update_dist_beta_alpha_cmd(group1, "UD_Beta_Alpha", ", def: 1.0]", {"UD_BALPHA", "update_distribution_beta_alpha"});
  args::ValueFlag<float> update_dist_beta_beta_cmd(group1, "UD_Beta_Beta", ", def: 1.0]", {"UD_BBETA", "update_distribution_beta_beta"});
  args::ValueFlag<float> update_dist_zipf_alpha_cmd(group1, "UD_Zipf_Alpha", ", def: 1.0]", {"UD_ZALPHA", "update_distribution_zipf_alpha"});

  args::ValueFlag<uint32_t> existing_point_lookup_dist_cmd(group1, "ED", "Existing Point Lookup Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"ED", "existing_point_lookup_distribution"});
  args::ValueFlag<float> existing_point_lookup_dist_norm_mean_percentile_cmd(group1, "ED_Norm_Mean_Percentile", ", def: 0.5]", {"ED_NMP", "existing_point_lookup_distribution_norm_mean_percentile"});
  args::ValueFlag<float> existing_point_lookup_dist_norm_stddev_cmd(group1, "ED_Norm_Stddev", ", def: 1]", {"ED_NDEV", "existing_point_lookup_distribution_norm_standard_deviation"});
  args::ValueFlag<float> existing_point_lookup_dist_beta_alpha_cmd(group1, "ED_Beta_Alpha", ", def: 1.0]", {"ED_BALPHA", "existing_point_lookup_distribution_beta_alpha"});
  args::ValueFlag<float> existing_point_lookup_dist_beta_beta_cmd(group1, "ED_Beta_Beta", ", def: 1.0]", {"ED_BBETA", "existing_point_lookup_distribution_beta_beta"});
  args::ValueFlag<float> existing_point_lookup_dist_zipf_alpha_cmd(group1, "ED_Zipf_Alpha", ", def: 1.0]", {"ED_ZALPHA", "existing_point_lookup_distribution_zipf_alpha"});


  args::ValueFlag<uint32_t> non_existing_point_lookup_dist_cmd(group1, "ZD", "Zero-result Point Lookup Distribution [0: uniform, 1:normal, 2:beta, 3:zipf, def: 0]", {"ZD", "non_existing_point_lookup_distribution"});
  args::ValueFlag<float> non_existing_point_lookup_dist_norm_mean_percentile_cmd(group1, "ZD_Norm_Mean_Percentile", ", def: 0.5]", {"ZD_NMP", "non_existing_point_lookup_distribution_norm_mean_percentile"});
  args::ValueFlag<float> non_existing_point_lookup_dist_norm_stddev_cmd(group1, "ZD_Norm_Stddev", ", def: 1]", {"ZD_NDEV", "non_existing_point_lookup_distribution_norm_standard_deviation"});
  args::ValueFlag<float> non_existing_point_lookup_dist_beta_alpha_cmd(group1, "ZD_Beta_Alpha", ", def: 1.0]", {"ZD_BALPHA", "non_existing_point_lookup_distribution_beta_alpha"});
  args::ValueFlag<float> non_existing_point_lookup_dist_beta_beta_cmd(group1, "ZD_Beta_Beta", ", def: 1.0]", {"ZD_BBETA", "non_existing_point_lookup_distribution_beta_beta"});
  args::ValueFlag<float> non_existing_point_lookup_dist_zipf_alpha_cmd(group1, "ZD_Zipf_Alpha", ", def: 1.0]", {"ZD_ZALPHA", "non_existing_point_lookup_distribution_zipf_alpha"});


  try {
      parser.ParseCLI(argc, argv);
  }
  catch (args::Help&) {
      std::cout << parser;
      exit(0);
      // return 0;
  }
  catch (args::ParseError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  catch (args::ValidationError& e) {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }

  insert_count = insert_cmd ? args::get(insert_cmd) : 0;
  update_count = update_cmd ? args::get(update_cmd) : 0;
  point_delete_count = point_delete_cmd ? args::get(point_delete_cmd) : 0;
  range_delete_count = range_delete_cmd ? args::get(range_delete_cmd) : 0;
  range_delete_selectivity = range_delete_selectivity_cmd ? args::get(range_delete_selectivity_cmd) : 0;
  point_query_count = point_query_cmd ? args::get(point_query_cmd) : 0;
  range_query_count = range_query_cmd ? args::get(range_query_cmd) : 0;
  range_query_selectivity = range_query_selectivity_cmd ? args::get(range_query_selectivity_cmd) : 0;
  zero_result_point_delete_proportion = zero_result_point_delete_proportion_cmd ? args::get(zero_result_point_delete_proportion_cmd) : 0;
  zero_result_point_lookup_proportion = zero_result_point_lookup_proportion_cmd ? args::get(zero_result_point_lookup_proportion_cmd) : 0;
  if(point_query_count != 0 && ( zero_result_point_lookup_proportion < 0 || zero_result_point_lookup_proportion > 1)){
        std::cerr << "\033[0;31m Error: \033[0m The proportion of zero-result point lookups should be set between 0 and 1" << std::endl;
	return 1;
  }
  entry_size = entry_size_cmd ? args::get(entry_size_cmd) : 8;

  non_existing_point_query_count = floor(point_query_count*zero_result_point_lookup_proportion);
  //std::cout << "Zero-result queries:" << non_existing_point_query_count << std::endl;
  existing_point_query_count = point_query_count - non_existing_point_query_count; 

  float maximum_unique_non_existing_point_query_proportion = unique_zero_result_point_lookup_proportion_cmd ? args::get(unique_zero_result_point_lookup_proportion_cmd) : 0.5;
  maximum_unique_non_existing_point_query_count = round(non_existing_point_query_count*maximum_unique_non_existing_point_query_proportion);
  //std::cout << "Zero-result queries:" << maximum_unique_non_existing_point_query_count << std::endl;
  float maximum_unique_existing_point_query_proportion = maximum_unique_existing_point_lookup_proportion_cmd ? args::get(maximum_unique_existing_point_lookup_proportion_cmd) : 0.5;
  maximum_unique_existing_point_query_count = round(existing_point_query_count*maximum_unique_existing_point_query_proportion);

  
  lambda = lambda_cmd ? args::get(lambda_cmd) : 0.5;
  if(lambda <= 0 || lambda > 1){
        std::cerr << "\033[0;31m ERROR:\033[0m Lambda should be set between 0 and 1" << std::endl;
	return 1;
  }
  if(!STRING_KEY_ENABLED && (lambda > 0 && lambda < 1)){
	key_size = sizeof(uint32_t);
  }else if(lambda > 0 && lambda < 1){
        key_size = lambda * entry_size;
  }

  load_from_existing_workload = load_from_existing_workload_cmd ? true : false;
  out_filename = out_filename_cmd ? args::get(out_filename_cmd) : "";


   // distribution
   insert_dist = insert_dist_cmd ? args::get(insert_dist_cmd):0;
   insert_norm_mean_percentile = insert_dist_norm_mean_percentile_cmd ? args::get(insert_dist_norm_mean_percentile_cmd):0.5;
   insert_norm_stddev = insert_dist_norm_stddev_cmd ? args::get(insert_dist_norm_stddev_cmd):1.0;
   insert_beta_alpha =  insert_dist_beta_alpha_cmd ? args::get(insert_dist_beta_alpha_cmd):1.0;
   insert_beta_beta =  insert_dist_beta_beta_cmd ? args::get(insert_dist_beta_beta_cmd):1.0;
   insert_zipf_alpha =  insert_dist_zipf_alpha_cmd ? args::get(insert_dist_zipf_alpha_cmd):1.0;
   

   update_dist = update_dist_cmd ? args::get(update_dist_cmd):0;
   update_norm_mean_percentile = update_dist_norm_mean_percentile_cmd ? args::get(update_dist_norm_mean_percentile_cmd):0.5;
   update_norm_stddev = update_dist_norm_stddev_cmd ? args::get(update_dist_norm_stddev_cmd):1.0;
   update_beta_alpha =  update_dist_beta_alpha_cmd ? args::get(update_dist_beta_alpha_cmd):1.0;
   update_beta_beta =  update_dist_beta_beta_cmd ? args::get(update_dist_beta_beta_cmd):1.0;
   update_zipf_alpha =  update_dist_zipf_alpha_cmd ? args::get(update_dist_zipf_alpha_cmd):1.0;
  

   non_existing_point_lookup_dist = non_existing_point_lookup_dist_cmd ? args::get(non_existing_point_lookup_dist_cmd):0;
   non_existing_point_lookup_norm_mean_percentile = non_existing_point_lookup_dist_norm_mean_percentile_cmd ? args::get(non_existing_point_lookup_dist_norm_mean_percentile_cmd):0.5;
   non_existing_point_lookup_norm_stddev = non_existing_point_lookup_dist_norm_stddev_cmd ? args::get(non_existing_point_lookup_dist_norm_stddev_cmd):1.0;
   non_existing_point_lookup_beta_alpha =  non_existing_point_lookup_dist_beta_alpha_cmd ? args::get(non_existing_point_lookup_dist_beta_alpha_cmd):1.0;
   non_existing_point_lookup_beta_beta =  non_existing_point_lookup_dist_beta_beta_cmd ? args::get(non_existing_point_lookup_dist_beta_beta_cmd):1.0;
   non_existing_point_lookup_zipf_alpha =  non_existing_point_lookup_dist_zipf_alpha_cmd ? args::get(non_existing_point_lookup_dist_zipf_alpha_cmd):1.0;
 
   existing_point_lookup_dist = existing_point_lookup_dist_cmd ? args::get(existing_point_lookup_dist_cmd):0;
   existing_point_lookup_norm_mean_percentile = existing_point_lookup_dist_norm_mean_percentile_cmd ? args::get(existing_point_lookup_dist_norm_mean_percentile_cmd):0.5;
   existing_point_lookup_norm_stddev = existing_point_lookup_dist_norm_stddev_cmd ? args::get(existing_point_lookup_dist_norm_stddev_cmd):1.0;
   existing_point_lookup_beta_alpha =  existing_point_lookup_dist_beta_alpha_cmd ? args::get(existing_point_lookup_dist_beta_alpha_cmd):1.0;
   existing_point_lookup_beta_beta = existing_point_lookup_dist_beta_beta_cmd ? args::get(existing_point_lookup_dist_beta_beta_cmd):1.0;
   existing_point_lookup_zipf_alpha =  existing_point_lookup_dist_zipf_alpha_cmd ? args::get(existing_point_lookup_dist_zipf_alpha_cmd):1.0;

   if(insert_norm_mean_percentile <= 0 || insert_norm_mean_percentile > 1 || update_norm_mean_percentile <= 0 || update_norm_mean_percentile > 1 || 
      non_existing_point_lookup_norm_mean_percentile <= 0 || non_existing_point_lookup_norm_mean_percentile > 1 || 
      existing_point_lookup_norm_mean_percentile <= 0 || existing_point_lookup_norm_mean_percentile > 1 
       ){
        std::cerr << "\033[0;31m ERROR:\033[0m The percentile of mean in normal distribution should be set between 0 and 1" << std::endl;
	return 1;
   }
//   std::cout << "in arg_parse, lambda = " << lambda << std::endl;

    return 0;
}

inline void showProgress(const uint32_t &workload_size, const uint32_t &counter) {
    // std::cout << "flag = " << flag << std::endl;

    if (counter / (workload_size/100) >= 1) {
        for (int i = 0; i<104; i++){
        std::cout << "\b";
        fflush(stdout);
        }
    }
    for (int i = 0; i<counter / (workload_size/100); i++){
        std::cout << "=" ;
        fflush(stdout);
    }
    std::cout << std::setfill(' ') << std::setw(101 - counter / (workload_size/100));
    std::cout << counter*100/workload_size << "%";
        fflush(stdout);

    if (counter == workload_size) {
        std::cout << "\n";
        return;
    }
}
