#include "mini_redis/Database.h"
#include <iostream>

using namespace std;

int main() {
    Database db;

    db.set("name", "hieu");
    db.set("language", "cpp");

    cout << "name = " << db.get("name") << endl;
    cout << "language = " << db.get("language") << endl;
    cout << "missing = " << db.get("missing") << endl;

    db.del("name");

    cout << "name after delete = " << db.get("name") << endl;

    return 0;
}