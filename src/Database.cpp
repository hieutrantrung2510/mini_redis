#include "mini_redis/Database.h"

using namespace std;

void Database::set(const string& key, const string& value) {
    store[key] = value;
}

string Database::get(const string& key) const {
    auto it = store.find(key);

    if (it == store.end()) {
        return "(nil)";
    }

    return it->second;
}

bool Database::del(const string& key) {
    auto it = store.find(key);

    if (it == store.end()) {
        return false;
    }

    store.erase(it);
    return true;
}