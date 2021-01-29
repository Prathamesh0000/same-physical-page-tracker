#include <iostream>
#include "absl/strings/str_cat.h"
#include "absl/container/flat_hash_map.h"

// https://abseil.io/blog/20180927-swisstables
absl::flat_hash_map<int, int> pfn_map;

int find(int key);

int main(int argc, const char** argv) {

	for( int i =0; i < 10000; i++) {
  		pfn_map.insert({i, i+100});
	}
	
	for( int i =0; i < 10000; i++) {
		// std::cout << find(i)  << "\n";
	}

}

int find(int key) {
	int value = -1;
	const auto &it = pfn_map.find(key);
	if (it != pfn_map.end()) {
		value = it->second;
		// std::cout << it->second << "\n";
	}
	return value;
}
