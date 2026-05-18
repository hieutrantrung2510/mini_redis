#include "mini_redis/TcpServer.h"

using namespace std;

int main() {
    TcpServer server(6380);

    if (!server.start()) {
        return 1;
    }

    server.run();

    return 0;
}