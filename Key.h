#ifndef KEY_H
#define KEY_H

#include <random>
#include <iostream>
#include <cstring>

#define INTEGER_KEY_DOMAIN 1073741824 // 2^30
#define STRING_PREFIX_DIGITS 2
using namespace std;

class Key{
public:
	bool string_enabled_;	
	bool empty_flag_;
	string key_str_;
	uint32_t key_int32_;	
	static const char key_alphanum[];
	Key();
	Key(string key);
	Key(uint32_t key);
	void SetEmpty();
	bool operator <(const Key & t) const;
	bool operator ==(const Key & t) const;
	Key operator +(const Key & t);
	friend ostream & operator <<(ostream & os, const Key& t);
	static Key get_key(int key_size, bool string_enabled=true);
	static Key get_key_smaller_than(const Key& key);
	static Key get_key_larger_than(const Key& key);
	static Key get_key_between(const Key& key1, const Key& key2);
};
#endif
