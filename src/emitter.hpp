#include <winsock2.h>
#include <time.h>

class Emitter
{
public:
    Emitter();
    Emitter(char *address, int port);
    void Begin();
    void Emit(char r, char g, char b);
    void End();

    char *GetAddress();
    int GetPort();

private:
    sockaddr_in serverAddress;
    int serverSocket;
};