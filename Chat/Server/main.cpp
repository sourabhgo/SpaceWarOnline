
#include <iostream>
#include <conio.h>
#include "net.h"
using namespace std;

const int BUFSIZE = 256;
const USHORT DEFAULT_PORT = 48161;
int main()
{
    USHORT port = DEFAULT_PORT;
    USHORT remotePort = DEFAULT_PORT;  // Remote port number
    int newPort = DEFAULT_PORT;
    int protocol;           // TCP or UDP
    char remoteIP[16];      // Remote IP address as dotted quad; nnn.nnn.nnn.nnn
    char localIP[16];       // Local IP address as dotted quad; nnn.nnn.nnn.nnn
    char netbuf[BUFSIZE];   // network receive
    char keybuf[BUFSIZE];   // keyboard input
    int error     = netNS::NET_OK; 
    int lastError = netNS::NET_OK; 
    int sizeXmit=0;         // transmit size
    int sizeRecv=0;         // receive size
    int size=0;
    Net net;                // Network communication object
    netbuf[0] = '\0';

    // ----- create server -----
    do
    {
        // display protocol menu
        cout << "----- Chat Server -----\n"
             << "\nSelect Protocol\n"
             << "    0 = UDP\n"
             << "    1 = TCP\n\n"
             << "  Choice: ";
        cin >> protocol;    // get character
    }while(protocol != 0 && protocol != 1);
    cin.ignore();

    // get port number
    //     Port numbers 0-1023 are used for well-known services.
    //     Port numbers 1024-65535 may be freely used.
    do
    {
        cout << "Enter port number (Use 0 for default 48161): ";
        cin >> newPort;
    }while(newPort < 0 || newPort > 65535);
    if(newPort != 0)
        port = newPort;
    cin.ignore();

    // create server
    error = net.createServer(port, protocol);
    if(error != netNS::NET_OK)          // if error
    {
        cout << net.getError(error) << endl;
        system("pause");
        return 1;
    }

    // display serverIP
    net.getLocalIP(localIP);
    cout << "Server IP is: " << localIP << endl;
    cout << "Server port is: " << port << endl;

    // display incoming text and send back response
    while(true)     // ***** INFINITE LOOP *****
    {
        // check for receive
        sizeRecv = BUFSIZE;     // max bytes to receive
        error = net.readData(netbuf,sizeRecv,remoteIP, remotePort);
        if(error != netNS::NET_OK)
        {
            cout << net.getError(error) << endl;
            error = net.closeSocket();     // close connection
            // re-create server
            error = net.createServer(port, protocol);
            if(error != netNS::NET_OK)          // if error
            {
                cout << net.getError(error) << endl;
                return 1;
            }
        }
        if(sizeRecv > 0)        // if characters received
        {
            // display incomming message
            cout << netbuf << endl;
        }

        // if keyboard input pending
        if (_kbhit())           
        {
            cin.getline(keybuf, BUFSIZE);    // get input
            sizeXmit += (int)cin.gcount() + 1;   // get size of input + null
            if(cin.fail())      // if failbit
            {
                cin.clear();    // clear error
                cin.ignore(INT_MAX, '\n');   // flush input
            }
        }

        // if data ready to send
        if(sizeXmit > 0)
        {
            size = sizeXmit;
            error = net.sendData(keybuf,size,remoteIP,remotePort); // send data
            if(error != netNS::NET_OK)
                cout << net.getError(error) << endl;
            sizeXmit -= size;   // - number of characters sent
            if(sizeXmit < 0)
                sizeXmit = 0;
        }

        // insert 10 millisecond delay to reduce CPU workload
        Sleep(10);
    }

    return 0;
}

