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
    remotePort = netNS::DEFAULT_PORT;
    netTime = 0;
    playerN = 0;                // assigned by server
    error     = netNS::NET_OK; 
    lastError = netNS::NET_OK; 
    sizeXmit=0;                 // transmit size
    sizeRecv=0;                 // receive size
    size=0;
    tryToConnect = false;
    clientConnected = false;
    commErrors = 0;
    commWarnings = 0;
    buttonState = 0;
    soundState = 0;
    gameState = 0;
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

    // Start ships on opposite sides of planet in stable clockwise orbit
    ship[0].setX(GAME_WIDTH/4 - shipNS::WIDTH);
    ship[1].setX(GAME_WIDTH - GAME_WIDTH/4);
    ship[0].setY(GAME_HEIGHT/2 - shipNS::HEIGHT);
    ship[1].setY(GAME_HEIGHT/2);
    ship[0].setVelocity(VECTOR2(0,-shipNS::SPEED));
    ship[1].setVelocity(VECTOR2(0,shipNS::SPEED));
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

    countDownTimer = spacewarNS::COUNT_DOWN;
    countDownOn = true;
}

//=============================================================================
// Update all game items
//=============================================================================
void Spacewar::update()
{
    buttonState = 0;        // clear network button state

    if (menuOn)
    {
        if (input->anyKeyPressed())
        {
            menuOn = false;
            input->clearAll();
            tryToConnect = true;    // connect to game server
        }
    } 
    else if(countDownOn)
    {
        countDownTimer -= frameTime;
        if(countDownTimer <= 0)
        {
            countDownOn = false;
            console->hide();
        }
    } 
    else 
    {
        for (int i=0; i<MAX_PLAYERS; i++)       // for all players
        {
            if (playerN == i)       // if we are player i
                // if engine on
                if (input->isKeyDown(SHIP_FORWARD_KEY)  || input->getGamepadDPadUp(1)) 
                    buttonState |= FORWARD_BIT;

            ship[i].rotate(shipNS::NONE);
            // if turn ship left
            if (input->isKeyDown(SHIP_LEFT_KEY) || input->getGamepadDPadLeft(1))
                buttonState |= LEFT_BIT;
            // if turn ship right
            if (input->isKeyDown(SHIP_RIGHT_KEY) || input->getGamepadDPadRight(1))
                buttonState |= RIGHT_BIT;

            // if ship fire
            if (input->isKeyDown(SHIP_FIRE_KEY) || input->getGamepadA(1))
                buttonState |= FIRE_BIT;
            ship[i].gravityForce(&planet, frameTime);
            torpedo[i].gravityForce(&planet, frameTime);

            // Update the entities
            ship[i].update(frameTime);
            torpedo[i].update(frameTime);
        }
    }
    planet.update(frameTime);
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
{}

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
        console->print("connect - connect to game server");
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
    else if (command == "connect")
        tryToConnect = true;        // connect to game server
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
// Do network communications
//=============================================================================
void Spacewar::communicate(float frameTime)
{
    if(clientConnected)
        getInfoFromServer();    // get game state from server

    // calculate elapsed time for network communications
    netTime += frameTime;
    if(netTime < netNS::NET_TIME)      // if not time to communicate
        return;

    if(tryToConnect)            // if we are trying to connect to server
    {
        if(netTime < JOIN_TIME) // if not time to try
            return;
        connectToServer();      // attempt to connect to game server
        netTime -= JOIN_TIME;
        return;
    }
    else if(clientConnected)    // if connected
    {
        checkNetworkTimeout();  // check for disconnect from server
        sendInfoToServer();     // send player input to server
    }
    netTime -= netNS::NET_TIME;
}

//=============================================================================
// Attempt to connect to the game server.
// When a connection is established the game begins.
//=============================================================================
void Spacewar::connectToServer()
{
    static int step = 0;
    static float waitTime;  // seconds we have waited for a response from server
    int size;
    int newPort;
    std::string str;
    std::stringstream ss;

    if(clientConnected)     // if connected
    {
        tryToConnect = false;
        console->print("Currently connected");
        return;
    }

    switch(step)
    {
    case 0:
        console->print("----- Spacewar Client -----");
        console->print("Enter IP address or name of server:");
        console->clearInput();              // clear input text
        console->show();
        step = 1;
        break;

    case 1:
        str = console->getInput();
        if(str == "")                       // if no address entered
            return;
        strcpy_s(remoteIP, str.c_str());    // copy address to remoteIP
        console->clearInput();              // clear input text
        console->print("Enter port number, 0 selects default 48161: ");
        step = 2;
        break;

    case 2:
        str = console->getInput();
        if (str == "")
            return;
        newPort = atoi(str.c_str());
        if(newPort == 0)
            newPort = netNS::DEFAULT_PORT;
        if(newPort > netNS::MIN_PORT && newPort < 65536)
        {
            port = newPort;
            step = 3;
        }
        else
            console->print("Invalid port number");
        console->clearInput();
        break;

    case 3:
        // create UDP client
        error = net.createClient(remoteIP, port, netNS::UDP);
        if(error != netNS::NET_OK)          // if error
        {
            console->print(net.getError(error));    // display error message
            tryToConnect = false;
            step = 0;
            return;
        }
        // send request to join the server
        console->print("Attempting to connect with server."); // display message
        toServerData.playerN = 255;        // playerN=255 is request to join
        size = sizeof(toServerData);
        console->print("'Request to join' sent to server.");
        error = net.sendData((char*) &toServerData, size, remoteIP, port);
        console->print(net.getError(error));
        waitTime = 0;
        step = 4;
        break;

    case 4:
        waitTime+=netTime;      // add time since last call to connect
        // if we timed out with no reponse to our request to join
        if (waitTime > CONNECT_TIMEOUT)        
        {
            step = 3;               // send request again
            console->print("'Request to join' timed out.");
            //tryToConnect = false;
            return;
        }
        // read ConnectResponse from server
        size = sizeof(connectResponse);
        error = net.readData((char*)&connectResponse, size, remoteIP, remotePort);
        if (error == netNS::NET_OK)     // if read was OK
        {
            if(size == 0)   // if no data received
                return;
            // if the server sent back the proper ID then we are connected
            if (strcmp(connectResponse.response, netNS::SERVER_ID) == 0) 
            {
                if (connectResponse.number < MAX_PLAYERS)   // if valid player number
                {
                    playerN = connectResponse.number;       // set my player number
                    ss << "Connected as player number: " << playerN;
                    console->print(ss.str());
                    clientConnected = true;
                    commErrors = 0;
                    commWarnings = 0;
                } 
                else
                    console->print("Invalid player number received from server.");
            } 
            else if (strcmp(connectResponse.response, netNS::SERVER_FULL) == 0) 
                console->print("Server Full");
            else
            {
                console->print("Invalid ID from server. Server sent:");
                console->print(connectResponse.response);
            }
        }
        else        // read error
        {
            console->print(net.getError(error));
        }
        tryToConnect = false;
        step = 0;
    }
}

//=============================================================================
// Check for network timeout
//=============================================================================
void Spacewar::checkNetworkTimeout()
{
    if (!clientConnected)
        return;
    commErrors++;                         // increment timeout count
    if (commErrors > netNS::MAX_ERRORS)   // if communication timeout
    {
        clientConnected = false;
        console->print("***** Disconnected from server. *****");
        console->show();
    }
}

//=============================================================================
// Send the keypress codes to the server
//=============================================================================
void Spacewar::sendInfoToServer()
{
    int size;
    // prepare structure to be sent
    toServerData.buttons = buttonState;
    toServerData.playerN = playerN;
    // send data from client to server
    size = sizeof(toServerData);
    error = net.sendData((char*) &toServerData, size, remoteIP, remotePort);
}

//=============================================================================
// Get toClientData from server.
// called by client to get game state from server
//=============================================================================
void Spacewar::getInfoFromServer()
{
    int size;
    size = sizeof(toClientData);
    int readStatus = net.readData((char *)&toClientData, size, remoteIP, remotePort);
    if( readStatus == netNS::NET_OK && size > 0) 
    {
        for(int i=0; i<MAX_PLAYERS; i++)        // for all player positions
        {
            // load new data into each ship and torpedo
            ship[i].setNetData(toClientData.player[i].shipData);
            ship[i].setScore(toClientData.player[i].shipData.score);
            torpedo[i].setNetData(toClientData.player[i].torpedoData);
        }

        // Game state
        // Bit 0 = roundStart
        // Bits 1-7 reserved for future use
        if((toClientData.gameState & ROUND_START_BIT) &&
            countDownOn == false)
            roundStart();

        gameState = toClientData.gameState; // save new game state

        // Play sounds as indicated by server
        // Momentary sounds are indicated by a state change. With a 
        // state change from 0 to 1 or 1 to 0 the sound is played
        // one time. Other sounds are indicated with 1=on, 0=off.
        // Bit 0 = cheer        state change
        // Bit 1 = collide      state change
        // Bit 2 = explode      state change
        // Bit 3 = engine1      1=on, 0=off
        // Bit 4 = engine2      1=on, 0=off
        // Bit 5 = torpedoCrash state change
        // Bit 6 = torpedoFire  state change
        // Bit 7 = torpedoHit   state change
        if((toClientData.sounds & CHEER_BIT) != (soundState & CHEER_BIT))
            audio->playCue(CHEER);
        if((toClientData.sounds & COLLIDE_BIT) != (soundState & COLLIDE_BIT))
            audio->playCue(COLLIDE);
        if((toClientData.sounds & EXPLODE_BIT) != (soundState & EXPLODE_BIT))
            audio->playCue(EXPLODE);
        if(toClientData.sounds & ENGINE1_BIT)
            audio->playCue(ENGINE1);
        else
            audio->stopCue(ENGINE1);
        if(toClientData.sounds & ENGINE2_BIT)
            audio->playCue(ENGINE2);
        else
            audio->stopCue(ENGINE2);
        if((toClientData.sounds & TORPEDO_CRASH_BIT) != 
            (soundState & TORPEDO_CRASH_BIT))
            audio->playCue(TORPEDO_CRASH);
        if((toClientData.sounds & TORPEDO_FIRE_BIT) !=
            (soundState & TORPEDO_FIRE_BIT))
            audio->playCue(TORPEDO_FIRE);
        if((toClientData.sounds & TORPEDO_HIT_BIT) != 
            (soundState & TORPEDO_HIT_BIT))
            audio->playCue(TORPEDO_HIT);

        soundState = toClientData.sounds;      // save new sound states

        commErrors = 0;
        commWarnings = 0;
    }

}
