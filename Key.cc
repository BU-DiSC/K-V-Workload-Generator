#include "Key.h"


const char Key::key_alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
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
