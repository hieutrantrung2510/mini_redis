#include "mini_redis/Database.h"
#include <cassert>
#include <iostream>

using namespace std;

void test_set_and_get() {
    Database db;

    db.set("name", "hieu");

    assert(db.get("name") == "hieu");
}

void test_get_missing_key() {
    Database db;

    assert(db.get("missing") == "(nil)");
}

void test_delete_existing_key() {
    Database db;

    db.set("name", "hieu");

    bool deleted = db.del("name");

    assert(deleted == true);
    assert(db.get("name") == "(nil)");
}

void test_delete_missing_key() {
    Database db;

    bool deleted = db.del("missing");

    assert(deleted == false);
}

void test_overwrite_value() {
    Database db;

    db.set("name", "hieu");
    db.set("name", "trung");

    assert(db.get("name") == "trung");
}

int main() {
    test_set_and_get();
    test_get_missing_key();
    test_delete_existing_key();
    test_delete_missing_key();
    test_overwrite_value();

    cout << "All Database tests passed!" << endl;

    return 0;
}