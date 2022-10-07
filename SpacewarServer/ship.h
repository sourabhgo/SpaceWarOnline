
#ifndef _SHIP_H                 // Prevent multiple definitions if this 
#define _SHIP_H                 // file is included in more than one place
#define WIN32_LEAN_AND_MEAN

#include "entity.h"
#include "constants.h"

namespace shipNS
{
    const int   WIDTH = 32;                 // image width (each frame)
    const int   HEIGHT = 32;                // image height
    const int   X = GAME_WIDTH/2 - WIDTH/2; // location on screen
    const int   Y = GAME_HEIGHT/6 - HEIGHT;
    const float ROTATION_RATE = (float)PI; // radians per second
    const float SPEED = 100;                // 100 pixels per second
    const float MASS = 300.0f;              // mass
    enum DIRECTION {NONE, LEFT, RIGHT};     // rotation direction
    const int   TEXTURE_COLS = 8;           // texture has 8 columns
    const int   SHIP1_START_FRAME = 0;      // ship1 starts at frame 0
    const int   SHIP1_END_FRAME = 3;        // ship1 animation frames 0,1,2,3
    const int   SHIP2_START_FRAME = 8;      // ship2 starts at frame 8
    const int   SHIP2_END_FRAME = 11;       // ship2 animation frames 8,9,10,11
    const float SHIP_ANIMATION_DELAY = 0.2f;    // time between frames
    const int   EXPLOSION_START_FRAME = 32; // explosion start frame
    const int   EXPLOSION_END_FRAME = 39;   // explosion end frame
    const float EXPLOSION_ANIMATION_DELAY = 0.2f;   // time between frames
    const int   ENGINE_START_FRAME = 16;    // engine start frame
    const int   ENGINE_END_FRAME = 19;      // engine end frame
    const float ENGINE_ANIMATION_DELAY = 0.1f;  // time between frames
    const int   SHIELD_START_FRAME = 24;    // shield start frame
    const int   SHIELD_END_FRAME = 27;      // shield end frame
    const float SHIELD_ANIMATION_DELAY = 0.1f; // time between frames
    const float TORPEDO_DAMAGE = 46;        // amount of damage caused by torpedo
    const float SHIP_DAMAGE = 10;           // damage caused by collision with another ship
}

// ShipStc contains all of the information a network client needs for a ship.
struct ShipStc 
{
    float X;
    float Y;
    float radians;
    float health;
    VECTOR2 velocity;
    float rotation;             // rotation rate (radians/second)
    short score;
    UCHAR playerN;              // which player (255 is request to join)
    //-----flags-----
    // bit0 active              // true when player is active
    // bit1 engineOn
    // bit2 shieldOn
    UCHAR flags;                // boolean status flags
};

// inherits from Entity class
class Ship : public Entity
{
private:
    float   oldX, oldY, oldAngle;
    float   rotation;               // current rotation rate (radians/second)
    shipNS::DIRECTION direction;    // direction of rotation
    float   explosionTimer;
    int     score;
    bool    explosionOn;
    bool    engineOn;               // true to move ship forward
    bool    shieldOn;
    Image   engine;
    Image   shield;
    Image   explosion;

    // Network Variables
    char    netIP[16];      // IP address as dotted quad; nnn.nnn.nnn.nnn
    int     timeout;
    bool    connected;
    UCHAR   buttons;        // current key presses
                            // bit 0=Left, 1=Forward, 2=Right, 3=Fire
    u_long  packetNumber;   // used to detect out of order packets
    UCHAR   playerN;        // our player number
    int     commWarnings;   // count of communication warnings
    int     commErrors;     // count of communication errors

public:
    // constructor
    Ship();

    // inherited member functions
    virtual void draw();
    virtual bool initialize(Game *gamePtr, int width, int height, int ncols,
                            TextureManager *textureM);

    // update ship position and angle
    void update(float frameTime);

    // damage ship with WEAPON
    void damage(WEAPON);

    // new member functions
    
    // move ship out of collision
    void toOldPosition()            
    {
        spriteData.x = oldX; 
        spriteData.y = oldY, 
        spriteData.angle = oldAngle;
        rotation = 0.0f;
    }

    // Returns rotation
    float getRotation() {return rotation;}

    // Returns engineOn condition
    bool getEngineOn()  {return engineOn;}

    // Returns shieldOn condition
    bool getShieldOn()  {return shieldOn;}

    // Return score
    int getScore()      {return score;}

    // Return IP address of this player as dotted quad; nnn.nnn.nnn.nnn
    const char* getNetIP()  {return netIP;}

    // Return netTimeout count
    int getTimeout()    {return timeout;}

    // Return netConnected boolean
    bool getConnected() {return connected;}

    // Return buttons
    UCHAR getButtons()      {return buttons;}

    // Return packetNumber
    u_long getPacketNumber() {return packetNumber;}

    // Return Ship Data
    ShipStc getNetData();

    // Return player number
    int getPlayerN()        {return playerN;}

    // Return commWarnings
    int getCommWarnings()   {return commWarnings;}

    // Return commErrors
    int getCommErrors()     {return commWarnings;}

    // Return explosionOn
    bool getExplosionOn()   {return explosionOn;}

    // Sets engine on
    void setEngineOn(bool eng)  {engineOn = eng;}

    // Set shield on
    void setShieldOn(bool sh)   {shieldOn = sh;}

    // Sets Mass
    void setMass(float m)       {mass = m;}

    // Set rotation rate
    void setRotation(float r)   {rotation = r;}

    // Sets all ship data from ShipStc sent from server to client
    void setNetData(ShipStc ss);

    // Set score
    void setScore(int s)        {score = s;}

    // Set IP address of this player as dotted quad; nnn.nnn.nnn.nnn
    void setNetIP(const char* address)  {strcpy_s(netIP, address);}

    // Set netConnected boolean
    void setConnected(bool c)    {connected = c;}

    // Set buttons
    void setButtons(UCHAR b)        {buttons = b;}

    // Set packetNumber
    void setPacketNumber(u_long n)  {packetNumber = n;}

    // Set commWarnings
    void setCommWarnings(int w)     {commWarnings = w;}

    // Set commErrors
    void setCommErrors(int e)       {commErrors = e;}

    // Set timeout
    void setTimeout(int t)          {timeout = t;}

    // Increment commWarnings
    void incCommWarnings()          {commWarnings++;}

    // Increment commErrors
    void incCommErrors()            {commErrors++;}

    // Increment netTimeout count
    void incTimeout()               {timeout++;}

    // Add 1 to ship score
    void scored()                   {score++;}

    // direction of rotation force
    void rotate(shipNS::DIRECTION dir) {direction = dir;}

    // ship explodes
    void explode();

    // ship is repaired
    void repair();
};
#endif

