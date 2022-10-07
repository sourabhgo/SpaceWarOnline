
#ifndef _CONSTANTS_H            // Prevent multiple definitions if this 
#define _CONSTANTS_H            // file is included in more than one place
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

//-----------------------------------------------
// Useful macros
//-----------------------------------------------
// Safely delete pointer referenced item
#define SAFE_DELETE(ptr)       { if (ptr) { delete (ptr); (ptr)=NULL; } }
// Safely release pointer referenced item
#define SAFE_RELEASE(ptr)      { if(ptr) { (ptr)->Release(); (ptr)=NULL; } }
// Safely delete pointer referenced array
#define SAFE_DELETE_ARRAY(ptr) { if(ptr) { delete [](ptr); (ptr)=NULL; } }
// Safely call onLostDevice
#define SAFE_ON_LOST_DEVICE(ptr)    { if(ptr) { ptr->onLostDevice(); } }
// Safely call onResetDevice
#define SAFE_ON_RESET_DEVICE(ptr)   { if(ptr) { ptr->onResetDevice(); } }
#define TRANSCOLOR  SETCOLOR_ARGB(0,255,0,255)  // transparent color (magenta)

//-----------------------------------------------
//                  Constants
//-----------------------------------------------
// window
const char CLASS_NAME[] = "Spacewar";
const char GAME_TITLE[] = "Spacewar Network Client v2.1";
const bool FULLSCREEN = false;              // windowed or fullscreen
const UINT GAME_WIDTH =  640;               // width of game in pixels
const UINT GAME_HEIGHT = 480;               // height of game in pixels
 
// game
const double PI = 3.14159265358979;
const double PIx2 = PI*2.0;
const float FRAME_RATE = 200.0f;            // the target frame rate (frames/sec)
const float MIN_FRAME_RATE = 10.0f;             // the minimum frame rate
const float MIN_FRAME_TIME = 1.0f/FRAME_RATE;   // minimum desired time for 1 frame
const float MAX_FRAME_TIME = 1.0f/MIN_FRAME_RATE; // maximum time used in calculations
const float FULL_HEALTH = 100;

// graphic images
const char NEBULA_IMAGE[] =   "pictures\\orion.jpg";     // photo source NASA/courtesy of nasaimages.org 
const char TEXTURES_IMAGE[] = "pictures\\textures.png";  // game textures
const char MENU_IMAGE[] =     "pictures\\menu.png";      // menu texture

// audio files required by audio.cpp
// WAVE_BANK must be location of .xwb file.
const char WAVE_BANK[]  = "audio\\Win\\waveBank.xwb";
// SOUND_BANK must be location of .xsb file.
const char SOUND_BANK[] = "audio\\Win\\soundBank.xsb";

// audio cues
const char CHEER[]          = "cheer";
const char COLLIDE[]        = "collide";
const char EXPLODE[]        = "explode";
const char ENGINE1[]        = "engine1";
const char ENGINE2[]        = "engine2";
const char TORPEDO_CRASH[]  = "torpedoCrash";
const char TORPEDO_FIRE[]   = "torpedoFire";
const char TORPEDO_HIT[]    = "torpedoHit";

// key mappings
// In this game simple constants are used for key mappings. If variables were used
// it would be possible to save and restore key mappings from a data file.
const UCHAR CONSOLE_KEY  = VK_OEM_3;    // `~ key for U.S.
const UCHAR ESC_KEY      = VK_ESCAPE;   // escape key
const UCHAR ALT_KEY      = VK_MENU;     // Alt key
const UCHAR ENTER_KEY    = VK_RETURN;   // Enter key
const UCHAR SHIP_LEFT_KEY   = VK_LEFT;  // left arrow
const UCHAR SHIP_RIGHT_KEY  = VK_RIGHT; // right arrow
const UCHAR SHIP_FORWARD_KEY = VK_UP;   // up arrow
const UCHAR SHIP_FIRE_KEY    = VK_DOWN; // down arrow

// weapon types
enum WEAPON {TORPEDO, SHIP, PLANET};

#endif
