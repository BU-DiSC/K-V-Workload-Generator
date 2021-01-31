#include "Key.h"


const char Key::key_alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
Key::Key(){
  string_enabled_ = true;
  key_str_ = "";
}

Key::Key(string key){
  key_str_ = key;
  string_enabled_ = true;
}

Key::Key(uint32_t key){
  key_int32_ = key;
  string_enabled_ = false;
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
    char *s = new char[(int)_key_size];
    for (int i = 0; i < _key_size; ++i) {
        s[i] = key_alphanum[rand() % (sizeof(key_alphanum) - 1)];
    }
    s[_key_size] = '\0';
    return Key(string(s));

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
