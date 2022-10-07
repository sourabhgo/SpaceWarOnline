// This class is the core of the game

#include "spaceWar.h"
using namespace spacewarNS;

//=============================================================================
// Constructor
//=============================================================================
Spacewar::Spacewar()
{
    menuOn = true;
    countDownOn = false;
    initialized = false;
    port = netNS::DEFAULT_PORT;
    netTime = 0;
    startTimerRun = false;
    startTimer = 0;
    playerCount = 0;
    menuTimer = 0;
}

//=============================================================================
// Destructor
//=============================================================================
Spacewar::~Spacewar()
{
    releaseAll();           // call onLostDevice() for every graphics item
}

//=============================================================================
// Initializes the game
// Throws GameError on error
//=============================================================================
void Spacewar::initialize(HWND hwnd)
{
    Game::initialize(hwnd); // throws GameError

    // initialize DirectX fonts
    fontBig.initialize(graphics, spacewarNS::FONT_BIG_SIZE, false, false, spacewarNS::FONT);
    fontBig.setFontColor(spacewarNS::FONT_COLOR);
    fontScore.initialize(graphics, spacewarNS::FONT_SCORE_SIZE, false, false, spacewarNS::FONT);

    // menu texture
    if (!menuTexture.initialize(graphics,MENU_IMAGE))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing menu texture"));

    // nebula texture
    if (!nebulaTexture.initialize(graphics,NEBULA_IMAGE))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing nebula texture"));

    // main game textures
    if (!gameTextures.initialize(graphics,TEXTURES_IMAGE))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing game textures"));

    // menu image
    if (!menu.initialize(graphics,0,0,0,&menuTexture))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing menu"));

    // nebula image
    if (!nebula.initialize(graphics,0,0,0,&nebulaTexture))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing nebula"));

    // planet
    if (!planet.initialize(this, planetNS::WIDTH, planetNS::HEIGHT, 2, &gameTextures))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing planet"));

    // ship1
    if (!ship[0].initialize(this, shipNS::WIDTH, shipNS::HEIGHT, shipNS::TEXTURE_COLS, &gameTextures))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing ship1"));

    ship[0].setFrames(shipNS::SHIP1_START_FRAME, shipNS::SHIP1_END_FRAME);
    ship[0].setCurrentFrame(shipNS::SHIP1_START_FRAME);
    ship[0].setColorFilter(SETCOLOR_ARGB(255,230,230,255));   // light blue, used for shield and torpedo
    ship[0].setMass(shipNS::MASS);

    // ship2
    if (!ship[1].initialize(this, shipNS::WIDTH, shipNS::HEIGHT, shipNS::TEXTURE_COLS, &gameTextures))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing ship2"));

    ship[1].setFrames(shipNS::SHIP2_START_FRAME, shipNS::SHIP2_END_FRAME);
    ship[1].setCurrentFrame(shipNS::SHIP2_START_FRAME);
    ship[1].setColorFilter(SETCOLOR_ARGB(255,255,255,64));    // light yellow, used for shield
    ship[1].setMass(shipNS::MASS);

    // torpedo1
    if (!torpedo[0].initialize(this, torpedoNS::WIDTH, torpedoNS::HEIGHT, torpedoNS::TEXTURE_COLS, &gameTextures))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing torpedo1"));

    torpedo[0].setFrames(torpedoNS::START_FRAME, torpedoNS::END_FRAME);
    torpedo[0].setCurrentFrame(torpedoNS::START_FRAME);
    torpedo[0].setColorFilter(SETCOLOR_ARGB(255,128,128,255));   // light blue

    // torpedo2
    if (!torpedo[1].initialize(this, torpedoNS::WIDTH, torpedoNS::HEIGHT, torpedoNS::TEXTURE_COLS, &gameTextures))
        throw(GameError(gameErrorNS::FATAL_ERROR, "Error initializing torpedo2"));

    torpedo[1].setFrames(torpedoNS::START_FRAME, torpedoNS::END_FRAME);
    torpedo[1].setCurrentFrame(torpedoNS::START_FRAME);
    torpedo[1].setColorFilter(SETCOLOR_ARGB(255,255,255,64));     // light yellow

    // health bar
    healthBar.initialize(graphics, &gameTextures, 0, spacewarNS::HEALTHBAR_Y, 2.0f, graphicsNS::WHITE);

    toClientData.gameState = 0;
    toClientData.sounds = 0;
    initializeServer(port);     // initialize game server

    roundOver = true;
    return;
}

//=============================================================================
// Start a new round of play
//=============================================================================
void Spacewar::roundStart()
{
    // Start ships on opposite sides of planet in stable clockwise orbit
    ship[0].setX(GAME_WIDTH/4 - shipNS::WIDTH);
    ship[1].setX(GAME_WIDTH - GAME_WIDTH/4);
    ship[0].setY(GAME_HEIGHT/2 - shipNS::HEIGHT);
    ship[1].setY(GAME_HEIGHT/2);
    ship[0].setVelocity(VECTOR2(0,-shipNS::SPEED));
    ship[1].setVelocity(VECTOR2(0,shipNS::SPEED));

    ship[0].setDegrees(0);
    ship[1].setDegrees(180);
    ship[0].repair();
    ship[1].repair();
    countDownTimer = spacewarNS::COUNT_DOWN;
    countDownOn = true;
    roundOver = false;
    // set the state to indicate new round
    toClientData.gameState |= ROUND_START_BIT;
}

//=============================================================================
// Update all game items
//=============================================================================
void Spacewar::update()
{
    int shipCount = 0;      // visible ships
    playerCount = 0;

    if(menuOn)
    {
        menuTimer += frameTime;
        if(menuTimer >= MENU_TIME)
        {
            menuOn = false;
            console->show();
        }
    }

    if(startTimerRun)
    {
        startTimer += frameTime;
        if(startTimer >= START_TIME)
        {
            startTimer = 0;
            startTimerRun = false;
            roundStart();
        }
    }

    if(countDownOn)
    {
        countDownTimer -= frameTime;
        if(countDownTimer <= 0)
        {
            countDownOn = false;
            // round start bit off
            toClientData.gameState &= (0xFF ^ ROUND_START_BIT);
        }
    } 
    else
    {
        for (int i=0; i<MAX_PLAYERS; i++)       // for all players
        {
            if(ship[i].getConnected())
                playerCount++;  // count connected players

            if(ship[i].getVisible())
                shipCount++;        // count visible ships

            toClientData.sounds &= (0xFF ^ ENGINE1_BIT);    // sound off
            toClientData.sounds &= (0xFF ^ ENGINE2_BIT);    // sound off

            if (ship[i].getActive())
            {
                if (ship[i].getButtons() & FORWARD_BIT) // if move forward button
                {
                    ship[i].setEngineOn(true);
                    if(i==0)        // if ship1
                        toClientData.sounds |= ENGINE1_BIT; // sound on
                    else            // if ship2
                        toClientData.sounds |= ENGINE2_BIT; // sound on
                }
                else
                    ship[i].setEngineOn(false); // engine off

                ship[i].rotate(shipNS::NONE);
                if (ship[i].getButtons() & LEFT_BIT)    // if turn left button
                    ship[i].rotate(shipNS::LEFT);
                if (ship[i].getButtons() & RIGHT_BIT)   // if turn right button
                    ship[i].rotate(shipNS::RIGHT);

                if (ship[i].getButtons() & FIRE_BIT)    // if fire button
                {
                    torpedo[i].fire(&ship[i]);          // fire torpedo
                    if(torpedo[i].getFired())           // if it fired
                    {
                        // change the state of the sound bit to play the sound
                        toClientData.sounds ^= TORPEDO_FIRE_BIT;
                        torpedo[i].setFired(false);     // do not play sound again
                    }
                }
            }
            ship[i].gravityForce(&planet, frameTime);
            torpedo[i].gravityForce(&planet, frameTime);

            // Update the entities
            ship[i].update(frameTime);
            torpedo[i].update(frameTime);
        }
    }

    planet.update(frameTime);

    // if 2 or more players connected AND (only 1 visible OR round over)
    if( playerCount >= 2 && (shipCount <= 1 || roundOver))
        startTimerRun = true;
}

//=============================================================================
// Artificial Intelligence
//=============================================================================
void Spacewar::ai()
{}

//=============================================================================
// Handle collisions
//=============================================================================
void Spacewar::collisions()
{
    VECTOR2 collisionVector;
    UCHAR sounds = toClientData.sounds; // get current sound states

    for (int i=0; i<MAX_PLAYERS; i++)   // for all players
    {
        // if collision between ship and planet
        if(ship[i].collidesWith(planet, collisionVector))
        {
            ship[i].toOldPosition();    // move ship out of collision
            ship[i].damage(PLANET);
            for (int j=0; j<MAX_PLAYERS; j++) // for all ships
            {
                if(i != j)              // for all other ships
                    ship[j].scored();   // everyone else scores
            }
        }

        for (int j=i+1; j<MAX_PLAYERS; j++) // for all other ships
        {
            // if collision between ships
            if(ship[i].collidesWith(ship[j], collisionVector))
            {
                // bounce off other ship
                ship[i].bounce(collisionVector, ship[j]);
                ship[j].bounce(collisionVector*-1, ship[i]);
                ship[i].damage(SHIP);
                ship[j].damage(SHIP);
                if(ship[i].getHealth() <= 0)
                    ship[j].scored();
                if(ship[j].getHealth() <= 0)
                    ship[i].scored();
                // change the state of the sound bit to play the sound
                if(sounds & COLLIDE_BIT)    // if bit was 1
                    toClientData.sounds &= (0xFF ^ COLLIDE_BIT); // set to 0
                else                        // bit was 0
                    toClientData.sounds |= COLLIDE_BIT;     // set to 1
            }
        }

        for (int j=0; j<MAX_PLAYERS; j++)   // for all torpedos
        {
            if(i != j)  // don't collide with our own torpedo
            {
                // if collision between ship and torpedo
                if(ship[i].collidesWith(torpedo[j], collisionVector))
                {
                    ship[i].damage(TORPEDO);
                    torpedo[j].setVisible(false);
                    torpedo[j].setActive(false);
                    ship[j].scored();
                    // change the state of the sound bit to play the sound
                    if(sounds & TORPEDO_HIT_BIT)    // if bit was 1
                        toClientData.sounds &= (0xFF ^ TORPEDO_HIT_BIT); // set 0
                    else                            // bit was 0
                        toClientData.sounds |= TORPEDO_HIT_BIT;     // set 1
                }
            }
        }

        if(ship[i].getExplosionOn())
            toClientData.sounds ^= EXPLODE_BIT; // play explosion sound

        // if collision between torpedo and planet
        if(torpedo[i].collidesWith(planet, collisionVector))
        {
            torpedo[i].crash();
            // change the state of the sound bit to play the sound
            toClientData.sounds ^= TORPEDO_CRASH_BIT;
        }
    }
}

//=============================================================================
// Render game items
//=============================================================================
void Spacewar::render()
{
    graphics->spriteBegin();                // begin drawing sprites

    nebula.draw();                          // display orion nebula
    planet.draw();                          // draw the planet

    // display scores
    fontScore.setFontColor(spacewarNS::SHIP1_COLOR);
    _snprintf_s(buffer, spacewarNS::BUF_SIZE, "%d", ship[0].getScore());
    fontScore.print(buffer,spacewarNS::SCORE1_X,spacewarNS::SCORE_Y);
    fontScore.setFontColor(spacewarNS::SHIP2_COLOR);
    _snprintf_s(buffer, spacewarNS::BUF_SIZE, "%d", ship[1].getScore());
    fontScore.print(buffer,spacewarNS::SCORE2_X,spacewarNS::SCORE_Y);

    // display health bars
    healthBar.setX((float)spacewarNS::SHIP1_HEALTHBAR_X);
    healthBar.set(ship[0].getHealth());
    healthBar.draw(spacewarNS::SHIP1_COLOR);
    healthBar.setX((float)spacewarNS::SHIP2_HEALTHBAR_X);
    healthBar.set(ship[1].getHealth());
    healthBar.draw(spacewarNS::SHIP2_COLOR);

    ship[0].draw();                           // draw the spaceships
    ship[1].draw();

    torpedo[0].draw(graphicsNS::FILTER);      // draw the torpedos using colorFilter
    torpedo[1].draw(graphicsNS::FILTER);

    if(menuOn)
        menu.draw();
    if(countDownOn)
    {
        _snprintf_s(buffer, spacewarNS::BUF_SIZE, "%d", (int)(ceil(countDownTimer)));
        fontBig.print(buffer,spacewarNS::COUNT_DOWN_X,spacewarNS::COUNT_DOWN_Y);
    }

    graphics->spriteEnd();                  // end drawing sprites
}

//=============================================================================
// process console commands
//=============================================================================
void Spacewar::consoleCommand()
{
    command = console->getCommand();    // get command from console
    if(command == "")                   // if no command
        return;

    if (command == "help")              // if "help" command
    {
        console->print(" ");
        console->print("Console Commands:");
        console->print("~ - show/hide console");
        console->print("fps - toggle display of frames per second");
        console->print("gravity off - turns off planet gravity");
        console->print("gravity on - turns on planet gravity");
        console->print("port # - sets port number, CAUTION! Restarts server");
        return;
    }
    else if (command == "fps")
    {
        fpsOn = !fpsOn;                 // toggle display of fps
        if(fpsOn)
            console->print("fps On");
        else
            console->print("fps Off");
    }
    else if (command == "gravity off")
    {
        planet.setMass(0);
        console->print("Gravity Off");
    }
    else if (command == "gravity on")
    {
        planet.setMass(planetNS::MASS);
        console->print("Gravity On");
    }
    else if (command.substr(0,4) == "port")
    {
        int newPort = atoi(command.substr(5).c_str());
        if(newPort > netNS::MIN_PORT && newPort < 65536)
        {
            port = newPort;             // set new port
            countDownOn = false;
            playerCount = 0;
            netTime = 0;
            initializeServer(port);     // re-initialize game server
            roundOver = true;
        }
        else
            console->print("Invalid port number");
    }
}

//=============================================================================
// The graphics device was lost.
// Release all reserved video memory so graphics device may be reset.
//=============================================================================
void Spacewar::releaseAll()
{
    menuTexture.onLostDevice();
    nebulaTexture.onLostDevice();
    gameTextures.onLostDevice();
    fontScore.onLostDevice();
    fontBig.onLostDevice();

    Game::releaseAll();
    return;
}

//=============================================================================
// The grahics device has been reset.
// Recreate all surfaces.
//=============================================================================
void Spacewar::resetAll()
{
    fontBig.onResetDevice();
    fontScore.onResetDevice();
    gameTextures.onResetDevice();
    nebulaTexture.onResetDevice();
    menuTexture.onResetDevice();

    Game::resetAll();
    return;
}

////////////////////////////
//   Network Functions    //
////////////////////////////

//=============================================================================
// Initialize Server
//=============================================================================
int Spacewar::initializeServer(int port)
{
    std::stringstream ss;

    if(port < netNS::MIN_PORT)
    {
        console->print("Invalid port number");
        return netNS::NET_ERROR;
    }
    // ----- Initialize network stuff -----
    error = net.createServer(port, netNS::UDP);
    if(error != netNS::NET_OK)              // if error
    {
        console->print(net.getError(error));
        return netNS::NET_ERROR;
    }

    for (int i=0; i<MAX_PLAYERS; i++)       // for all players
    {
        ship[i].setActive(false);
        ship[i].setConnected(false);
        ship[i].setScore(0);
    }

    console->print("----- Server -----");
    net.getLocalIP(localIP);
    ss << "Server IP: " << localIP;
    console->print(ss.str());
    ss.str("");                             // clear stringstream
    ss << "Port: " << port;
    console->print(ss.str());
    return netNS::NET_OK;
}

//=============================================================================
// Do network communications
//=============================================================================
void Spacewar::communicate(float frameTime)
{
    // communicate with client
    // this function is not delayed so client response is as fast as possible
    doClientCommunication();

    // calculate elapsed time for network communications
    netTime += frameTime;
    if(netTime < netNS::NET_TIME)      // if not time to communicate
        return;
    netTime -= netNS::NET_TIME;

    // check for inactive clients, called every NET_TIME seconds
    checkNetworkTimeout();
}

//=============================================================================
// Check for network timeout
//=============================================================================
void Spacewar::checkNetworkTimeout()
{
    std::stringstream ss;

    for (int i=0; i<MAX_PLAYERS; i++)       // for all players
    {
        if (ship[i].getConnected())
        {
            ship[i].incTimeout();               // timeout++
            // if communication timeout
            if (ship[i].getTimeout() > netNS::MAX_ERRORS) 
            {
                ship[i].setConnected(false);
                ss << "***** Player " << i << " disconnected. *****";
                console->print(ss.str());
            }
        }
    }
}

//=============================================================================
// Do client communication
// Called by server to send game state to each client
//=============================================================================
void Spacewar::doClientCommunication()
{
    int playN;                  // player number we are communicating with
    int size;
    prepareDataForClient();     // prepare data for transmission to clients

    for (int i=0; i<MAX_PLAYERS; i++)   // for all players
    {
        size = sizeof(toServerData);
        if( net.readData((char*) &toServerData, size, remoteIP, port) == netNS::NET_OK) 
        {
            if(size > 0)                // if data received
            {
                playN = toServerData.playerN;
                if (playN == 255)       // if request to join game
                {
                    clientWantsToJoin();
                } 
                else if (playN >= 0 && playN < MAX_PLAYERS)  // if valid playerN
                {
                    if (ship[playN].getConnected()) // if this player is connected
                    {
                        if (ship[playN].getActive()) // if this player is active
                            ship[playN].setButtons(toServerData.buttons);
                        size = sizeof(toClientData);
                        // send player the latest game data
                        net.sendData((char*) &toClientData, size, remoteIP, port);
                        ship[playN].setTimeout(0);
                        ship[playN].setCommWarnings(0);
                    }
                }
            }
        } 
        else    // no more incomming data
        {
            break;
        }
    }
}

//=============================================================================
// Prepare data to send to client. It contains information on all players.
//=============================================================================
void Spacewar::prepareDataForClient()
{
    for (int i=0; i<MAX_PLAYERS; i++)       // for all players
    {
        toClientData.player[i].shipData = ship[i].getNetData();
        toClientData.player[i].torpedoData = torpedo[i].getNetData();
    }
}

//=============================================================================
// Client is requesting to join game
//=============================================================================
void Spacewar::clientWantsToJoin()
{
    std::stringstream ss;
    int size;
    int status;

    connectResponse.number = 255;       // set to invalid player number

    if(playerCount == 0)                // if no players currently in game
    {
        roundOver = true;               // start a new round
        for(int i=0; i<MAX_PLAYERS; i++)    // for all players
            ship[i].setScore(0);        // reset score
    }

    console->print("Player requesting to join.");
    // find available player position to use
    for(int i=0; i<MAX_PLAYERS; i++)        // search all player positions
    {
        if (ship[i].getConnected() == false)    // if this position available
        {
            ship[i].setConnected(true);
            ship[i].setTimeout(0);
            ship[i].setCommWarnings(0);
            ship[i].setNetIP(remoteIP);     // save player's IP
            ship[i].setCommErrors(0);       // clear old errors
            // send SERVER_ID and player number to client
            strcpy_s(connectResponse.response, netNS::SERVER_ID);
            connectResponse.number = (UCHAR)i;
            size = sizeof(connectResponse);
            status = net.sendData((char*)&connectResponse, size, remoteIP, port);
            if ( status == netNS::NET_ERROR) 
            {
                console->print(net.getError(status));   // display error message
                return;
            }
            toServerData.playerN = i;       // clear join request from input buffer                
            ss << "Connected player as number: " << i;
            console->print(ss.str());
            return;                         // found available player position
        }
    }
    // send SERVER_FULL to client
    strcpy_s(connectResponse.response, netNS::SERVER_FULL);
    size = sizeof(connectResponse);
    status = net.sendData((char*)&connectResponse, size, remoteIP, port);
    console->print("Server full.");
}


