#include "Key.h"
#include <vector>
#include <cassert>

using namespace std;

const char Key::key_alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

inline size_t getCharIndex(char c) {
  assert(c >= '0');
  assert(c <= 'z');
  if (c <= '9') {
    return c - '0';
  } else if (c <= 'Z') {
    return c - 'A' + 10;
  } else {
    return c - 'a' + 36;
  }
}
Key::Key(){
  string_enabled_ = true;
  empty_flag_ = true;
  key_str_ = "";
}

Key::Key(string key){
  key_str_ = key;
  string_enabled_ = true;
  empty_flag_ = false;
}

Key::Key(uint32_t key){
  key_int32_ = key;
  string_enabled_ = false;
  empty_flag_ = true;
}

void Key::SetEmpty() {
  empty_flag_ = true;
}

bool Key::operator < (const Key & t) const {
  if(string_enabled_){
    return key_str_ < t.key_str_;
  }else{
    return key_int32_ < t.key_int32_;
  }
}

bool Key::operator == (const Key & t) const {
  if (string_enabled_ != t.string_enabled_) return false;
  if(string_enabled_){
    return key_str_ == t.key_str_;
  }else{
    return key_int32_ == t.key_int32_;
  }
}

Key Key::operator +(const Key & t){
  if(string_enabled_){
    return Key(this->key_str_ + t.key_str_);
  }else{
    return Key(this->key_int32_ + t.key_int32_);
  }

}

Key Key::get_key(int _key_size, bool string_enabled){
  if(string_enabled){
    std::string s = std::string(_key_size, ' ');
    for (int i = 0; i < _key_size; ++i) {
        s[i] = key_alphanum[rand() % (sizeof(key_alphanum) - 1)];
    }
    return Key(s);

  } else{
    // here key_size means bits in fact  
    uint32_t domain_size = (uint32_t) (pow(2, _key_size) - 1);
    return Key((uint32_t) (rand()*rand()) % domain_size);
  }
}


ostream& operator<<(ostream & os, const Key& t){
  if(t.string_enabled_){
    os << t.key_str_;
  }else{
    os << t.key_int32_;
  }
  return os;
}

Key Key::get_key_smaller_than(const Key& key) {
  if(key.string_enabled_) {
    size_t pos = 0;
    char c;
    char new_c;
    size_t index_upper_bound = 0;
    std::string prefix = "";
    size_t key_string_size = key.key_str_.size();
    while (pos < key_string_size) {
    	c = key.key_str_[pos];
	if (c == '0') {
	  prefix.append(1, '0');
	  pos++;
	  continue;
	}
	index_upper_bound = getCharIndex(c);
	new_c = key_alphanum[rand() % (index_upper_bound + 1)];
	if (c < '4' && c == new_c) {
	   size_t k = 0; // re-generate four times if c is small to reduce the failure probability
	   while (k++ < 4 && c == new_c) new_c = key_alphanum[rand() % (index_upper_bound + 1)];
	}
	prefix.append(1, new_c);
	if (new_c < c) {
	  if (pos + 1 == key_string_size) {
	      return Key(prefix);
	  } else {
	      Key suffix_key = get_key(key_string_size - pos - 1, true);
	      return Key(prefix + suffix_key.key_str_); 
	  }
	} else { // new_c == c
	  pos++;
	}
    }
    // failed to generate a key smaller than the desired one
    prefix = std::string(key_string_size, '#');
    return Key(prefix); 
  } else {
    if (key.key_int32_ > RAND_MAX) {
    	return Key(rand() + rand()%(key.key_int32_ - RAND_MAX));
    } else {
    	return Key(rand()%key.key_int32_);
    }
  }
}

Key Key::get_key_larger_than(const Key& key) {
  if(key.string_enabled_) {
    size_t pos = 0;
    char c;
    char new_c;
    size_t index_lower_bound = 0;
    std::string prefix = "";
    size_t key_string_size = key.key_str_.size();
    while (pos < key_string_size) {
    	c = key.key_str_[pos];
	if (c == 'z') {
	  prefix.append(1, 'z');
	  pos++;
	  continue;
	}
	index_lower_bound = getCharIndex(c);
	new_c = key_alphanum[rand() % (sizeof(key_alphanum) - index_lower_bound - 1) + index_lower_bound];
	if (c > 'v' && c == new_c) {
	   size_t k = 0; // re-generate four times if c is small to reduce the failure probability
	   while (k++ < 4 && c == new_c) new_c = key_alphanum[rand() % (sizeof(key_alphanum) - index_lower_bound - 1) + index_lower_bound];
	}
	prefix.append(1, new_c);
	if (new_c > c) {
	  if (pos + 1 == key_string_size) {
	      return Key(prefix);
	  } else {
	      Key suffix_key = get_key(key_string_size - pos - 1, true);
	      return Key(prefix + suffix_key.key_str_); 
	  }
	} else { // new_c == c
	  pos++;
	}
    }
    // failed to generate a key larger than the desired one
    prefix = std::string(key_string_size, '|');
    return Key(prefix); 
  } else {
    if (key.key_int32_ > RAND_MAX) {
    	return Key(key.key_int32_ + rand()%(RAND_MAX - key.key_int32_%RAND_MAX));
    } else {
    	return Key(rand() + rand()%(RAND_MAX - key.key_int32_));
    }
  }
}


Key Key::get_key_between(const Key& keyA, const Key& keyB) {
   if (keyA.string_enabled_ != keyB.string_enabled_) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between two keys of different types" << std::endl;
	exit(0);
   }
   Key tmp;
   Key key1 = keyA;
   Key key2 = keyB;
   if (key2 < key1) {
       tmp = key2;
       key2 = key1;
       key1 = key2;
   }

   if (key1.string_enabled_) {
      size_t pos = 0;
      size_t key_string_size = key1.key_str_.size();
      if (key_string_size != key2.key_str_.size()) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between two keys of different length" << std::endl;
	exit(0);
      }
      if (key_string_size == 0) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between two empty strings" << std::endl;
	exit(0);
      }
      while (key1.key_str_[pos] == key2.key_str_[pos] && pos < key_string_size) pos++;
      if (pos >= key_string_size) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between two same strings" << std::endl;
	exit(0);
      }
      if (pos + 1 == key_string_size && (key1.key_str_[pos] + 1 == key2.key_str_[pos] ||
			      (key1.key_str_[pos] == '9' && key2.key_str_[pos] == 'A') || 
			      (key1.key_str_[pos] == 'Z' && key2.key_str_[pos] == 'a'))) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between two adjacent strings" << std::endl;
	exit(0);
      }

      char smaller_c = key1.key_str_[pos];
      char larger_c = key2.key_str_[pos];
      size_t index_lower_bound = getCharIndex(smaller_c);
      size_t index_upper_bound = getCharIndex(larger_c);
      char new_c = key_alphanum[rand()%(index_upper_bound - index_lower_bound + 1) + index_lower_bound];
      std::string new_key;
      if (pos > 0) {
	   new_key = key1.key_str_.substr(0, pos);
      }
      new_key.append(1, new_c);
      if (pos + 1 < key_string_size) {
	   std::string suffix;
	   Key suffix_key;
	   if (new_c == smaller_c) {
	     suffix_key = Key(key1.key_str_.substr(pos + 1));
	     tmp = get_key_larger_than(suffix_key);
           } else if (new_c == larger_c) {
	     suffix_key = Key(key2.key_str_.substr(pos + 1));
	     tmp = get_key_smaller_than(suffix_key);
           } else {
	     tmp = get_key(key_string_size -  pos - 1, true);   
           }
	   new_key.append(tmp.key_str_);
      }
      return Key(new_key); 
   } else {
      uint32_t diff = key2.key_int32_ - key1.key_int32_;
      if (diff == 0) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between the same keys" << std::endl;
	exit(0);
      }
      if (diff == 1) {
        std::cerr << "\033[0;31m Error: \033[0m Cannot generate a key between two adjacent keys" << std::endl;
	exit(0);
      }
      if (diff > RAND_MAX) {
	   return Key(rand() + rand()%(diff - RAND_MAX) + key1.key_int32_);
      } else {
	   return Key(rand()%diff + key1.key_int32_);
      }
   }

}
