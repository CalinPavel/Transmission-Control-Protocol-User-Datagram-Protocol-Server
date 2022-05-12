#define BUF 1500
#include <string>
#include <vector>
#include <list>
//used for udp datagram receiving
struct UdpPacket
{
    char topic[50];
    int8_t type;
    char str[BUF];
};
//used for SF and topic identification
struct Topic
{
    char name[BUF];
    char *sf;
    std::vector<std::string> storage;
};
//Struct used for tcp packets sending
struct TcpPacket
{
    int len;
    char msg[1500];
};
//Struct for users
struct User
{
    char id[10];
    std::vector<Topic> topics;
    int soket;
    int connected;
};
