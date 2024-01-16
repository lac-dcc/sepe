#include <iostream>

#include "absl/container/flat_hash_map.h"

int main(){
    // absl::flat_hash_set<std::string> set1;
    absl::flat_hash_map<int, std::string> map1;
    map1[1] = "one";
    map1[2] = "two";
    std::cout << map1[1] << std::endl;
}
