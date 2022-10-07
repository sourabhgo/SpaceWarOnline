
#ifndef _SPACEWAR_H             // Prevent multiple definitions if this 
#define _SPACEWAR_H             // file is included in more than one place
#define WIN32_LEAN_AND_MEAN

#include <string>
#include <sstream>
#include "game.h"
#include "textureManager.h"
#include "image.h"
#include "dashboard.h"
#include "planet.h"
#include "ship.h"
#include "torpedo.h"
#include "net.h"

namespace spacewarNS
{
    const char FONT[] = "Arial Bold";  // font
    const int FONT_BIG_SIZE = 256;     // font height
    const int FONT_SCORE_SIZE = 48;
    const COLOR_ARGB FONT_COLOR = graphicsNS::YELLOW;
    const COLOR_ARGB SHIP1_COLOR = graphicsNS::BLUE;
    const COLOR_ARGB SHIP2_COLOR = graphicsNS::YELLOW;
    const int SCORE_Y = 10;
    const int SCORE1_X = 60;
    const int SCORE2_X = GAME_WIDTH-80;
    const int HEALTHBAR_Y = 30;
    const int SHIP1_HEALTHBAR_X = 40;
    const int SHIP2_HEALTHBAR_X = GAME_WIDTH-100;
    const int HEALTHBAR_START_FRAME = 22;
    const int HEALTHBAR_END_FRAME = 22;
    const int HEALTHBAR_HEIGHT = 32;
    const int HEALTHBAR_WIDTH = 64;
    const int COUNT_DOWN_X = GAME_WIDTH/2 - FONT_BIG_SIZE/4;
    const int COUNT_DOWN_Y = GAME_HEIGHT/2 - FONT_BIG_SIZE/2;
    const int COUNT_DOWN = 5;           // count down from 5
    const int BUF_SIZE = 20;
    const int ROUND_TIME = 5;           // time until new round starts
    const int HEALTHBAR_TEXTURE_COLS = 4;   // health bar is a 4 column texture
    // Game types
    const int LOCAL = 0;            // two players on same computer
    const int CLIENT = 1;           // client in network game
    const int SERVER = 2;           // server in network game
    const int MAX_PLAYERS = 2;      // maximum number of network players
    // Network
    const int BUFSIZE = 256;
    const int LEFT_BIT = 0x01;      // player buttons
    const int FORWARD_BIT = 0x02;
    const int RIGHT_BIT = 0x04;
    const int FIRE_BIT = 0x08;
    const int CONNECT_TIMEOUT = 10; // seconds to wait when connecting
    const int JOIN_TIME = 5;        // seconds between join attempts
    // Game State bits
    const int ROUND_START_BIT = 0x01;
    // Network, sounds for client to play
    const int ENGINE1_BIT       = 0x01; // Bit 0 = engine1      1=on, 0=off
    const int ENGINE2_BIT       = 0x02; // Bit 1 = engine2      1=on, 0=off
    const int CHEER_BIT         = 0x04; // Bit 2 = cheer        state change
    const int COLLIDE_BIT       = 0x08; // Bit 3 = collide      state change
    const int EXPLODE_BIT       = 0x10; // Bit 4 = explode      state change
    const int TORPEDO_CRASH_BIT = 0x20; // Bit 5 = torpedoCrash state change
    const int TORPEDO_FIRE_BIT  = 0x40; // Bit 6 = torpedoFire  state change
    const int TORPEDO_HIT_BIT   = 0x80; // Bit 7 = torpedoHit   state change
}

//=============================================================================
// network play structures
//=============================================================================

// Sent to client in response to connection request
struct ConnectResponse
{
    char response[netNS::RESPONSE_SIZE];    // server response
    UCHAR   number;                         // player number if connected
};

// Player describes the ship and torpedo for one player.
struct Player
{
    ShipStc     shipData;
    TorpedoStc  torpedoData;
};

// ToClientStc is the structure that is sent from the server to each client. 
struct ToClientStc 
{
    Player  player[spacewarNS::MAX_PLAYERS];
    // game state
    // Bit 0 = roundStart
    // Bits 1-7 reserved for future use
    UCHAR   gameState;
    // sound to play   
    // Bit 0 = cheer        state change
    // Bit 1 = collide      state change
    // Bit 2 = explode      state change
    // Bit 3 = engine1      1=on, 0=off
    // Bit 4 = engine2      1=on, 0=off
    // Bit 5 = torpedoCrash state change
    // Bit 6 = torpedoFire  state change
    // Bit 7 = torpedoHit   state change
    UCHAR   sounds;
};

// ToServerStc is the structure that is sent from the client to the server.
struct ToServerStc 
{
    // current key presses
    UCHAR buttons;      // bit 0=Left, 1=Forward, 2=Right, 3=Fire
    UCHAR playerN;      // player number
};

//=============================================================================
// Spacewar is the class we create, it inherits from the Game class
//=============================================================================
class Spacewar : public Game
{
private:
    // game items
    TextureManager menuTexture, nebulaTexture, gameTextures;   // textures
    Ship    ship[spacewarNS::MAX_PLAYERS];      // spaceships
    Torpedo torpedo[spacewarNS::MAX_PLAYERS];   // torpedos
    Planet  planet;             // the planet
    Image   nebula;             // backdrop image
    Image   menu;               // menu image
    Bar     healthBar;          // health bar for ships
    bool    menuOn;
    bool    countDownOn;        // true when count down is displayed
    float   countDownTimer;
    TextDX  fontBig;            // DirectX font for game banners
    TextDX  fontScore;
    char    buffer[spacewarNS::BUF_SIZE];
    bool    roundOver;          // true when round is over
    float   roundTimer;         // time until new round starts
    int     ship1Score, ship2Score; // scores
    int     playerN;            // our player number

    // Network variables
    Net  net;                   // network object
    char protocol;              // TCP or UDP
    char remoteIP[spacewarNS::BUFSIZE];     // Remote IP address or hostname
    char localIP[16];           // Local IP address as dotted quad; nnn.nnn.nnn.nnn
    int  port;                  // port number
    USHORT remotePort;          // remote port number
    char netbuf[spacewarNS::BUFSIZE];       // network receive
    char keybuf[spacewarNS::BUFSIZE];       // keyboard input
    int error; 
    int lastError; 
    int sizeXmit;               // transmit size
    int sizeRecv;               // receive size
    int size;
    ToClientStc toClientData;   // data struct sent to client from server
    ToServerStc toServerData;   // data struct sent to server from client
    ConnectResponse connectResponse;
    UINT commErrors;
    UINT commWarnings;
    bool tryToConnect;
    bool clientConnected;
    UCHAR buttonState;          // current state of player buttons
    UCHAR soundState;           // current sound state
    UCHAR gameState;            // current game state
    
    float netTime;              // network timer

public:
    // Constructor
    Spacewar();
    // Destructor
    virtual ~Spacewar();
    // Initialize the game
    void initialize(HWND hwnd);
    void update();      // must override pure virtual from Game
    void ai();          // "
    void collisions();  // "
    void render();      // "
    void consoleCommand(); // process console command
    void roundStart();  // start a new round of play
    void releaseAll();
    void resetAll();

    // Network functions
    void communicate(float frameTime);
    void checkNetworkTimeout();
    void prepareDataForClient();
    void doClientCommunication();
    void sendInfoToServer();
    void getInfoFromServer();
    void clientWantsToJoin();
    void connectToServer();
};

#endif
