
#ifndef _TORPEDO_H              // Prevent multiple definitions if this 
#define _TORPEDO_H              // file is included in more than one place
#define WIN32_LEAN_AND_MEAN

#include "entity.h"
#include "constants.h"

namespace torpedoNS
{
    const int   WIDTH = 32;             // image width
    const int   HEIGHT = 32 ;           // image height
    const int   COLLISION_RADIUS = 4;   // for circular collision
    const float SPEED = 200;            // pixels per second
    const float MASS = 300.0f;          // mass
    const float FIRE_DELAY = 4.0f;      // 4 seconds between torpedo firing
    const int   TEXTURE_COLS = 8;       // texture has 8 columns
    const int   START_FRAME = 40;       // starts at frame 40
    const int   END_FRAME = 43;         // animation frames 40,41,42,43
    const float ANIMATION_DELAY = 0.1f; // time between frames
}

// TorpedoStc contains all of the information a client needs for one torpedo.
struct TorpedoStc 
{
    float X;
    float Y;
    VECTOR2 velocity;
    bool  active;                       // true when active
};

// inherits from Entity class
class Torpedo : public Entity
{
private:
    float   fireTimer;                  // time remaining until fire enabled
    bool    fired;
public:
    // constructor
    Torpedo();

    // inherited member functions
    void update(float frameTime);
    float getMass() const   {return torpedoNS::MASS;}

    // Return fired bool
    bool getFired() const   {return fired;}

    // Return torpedo data
    TorpedoStc getNetData();

    // Set fired bool
    void setFired(bool f)   {fired = f;}

    // new member functions
    void fire(Entity *ship);                // fire torpedo from ship
    void crash();                           // crash into planet

    // Sets all torpedo data from TorpedoStc sent from server to client
    void setNetData(TorpedoStc ts);
};
#endif

