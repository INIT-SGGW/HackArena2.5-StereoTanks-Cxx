# MonoTanks API wrapper in C++ for HackArena 2.5

This API wrapper for StereoTanks game for the HackArena 2.5, organized by
KN init. It is implemented as a WebSocket client written in C++ programming
language and can be used to create bots for the game.

To fully test and run the game, you will also need the game server and GUI
client, as the GUI provides a visual representation of gameplay. You can find
more information about the server and GUI client in the following repository:

- [Server and GUI Client Repository](https://github.com/INIT-SGGW/HackArena2.5-StereoTanks)

The guide to the game mechanics and tournament rules can be found on the:
- [Instruction Page](https://hackarena.pl/Assets/Game/HackArena%202.0%20-%20instrukcja.pdf)

## Development

Clone this repo using git:
```sh
git clone https://github.com/INIT-SGGW/HackArena2.5-StereoTanks.git
```

or download the [zip file](https://github.com/INIT-SGGW/HackArena2.5-StereoTanks/archive/refs/heads/main.zip)
and extract it.

The bot logic you are going to implement is located in `src/bot/bot.cpp` and declarations in `src/bot/bot.h`

The `Bot` class implements methods, which define the bot's
behavior. The constructor is called when the bot is created, and the
`NextMove` function is called every game tick to determine the bot's next
move. The `OnGameEnded` function is called when the game ends to provide the
final game state.

`NextMove` returns a `ResponseVariant` (`std::variant`), which can be one of the following:

- `Rotate`: This variant indicates a rotation action, allowing the player to rotate their tank and/or turret.
    - `tankRotation` (`RotationDirection`): Specifies the rotation direction of the tank (left, right, or none).
    - `turretRotation` (`RotationDirection`): Specifies the rotation direction of the turret (left, right, or none).

- `Move`: This variant indicates a movement action, allowing the player to move the tank.
    - `direction` (`MoveDirection`): Specifies the direction of movement (forward or backward).

- `AbilityUse`: This variant represents an ability use action, where the tank can use one of the following abilities:
    - `fireBullet`: Shoots a regular bullet in the direction the turret is pointing
    - `useLaser`: Uses a laser ability (heavy tanks only)
    - `fireDoubleBullet`: Fires two bullets simultaneously (light tanks only)
    - `useRadar`: Activates the radar for enhanced visibility (light tanks only)
    - `dropMine`: Drops a mine behind the tank (heavy tanks only)
    - `fireHealingBullet`: Fires a bullet that can heal friendly tanks
    - `fireStunBullet`: Fires a bullet that can temporarily stun enemy tanks

- `Wait`: This variant indicates that the player chooses to wait, doing nothing during the current tick.

- `CaptureZone`: This variant indicates that the player is attempting to capture a zone they are currently in.

- `GoTo`: This variant provides pathfinding capabilities for the tank:
    - `x`, `y`: Target coordinates to move to
    - `turretRotation` (optional): Direction to rotate the turret while moving
    - `costs` (optional): Custom movement costs for pathfinding
    - `penalties` (optional): Custom penalties for different obstacles

`ResponseVariant` allows the game to handle different player actions in a flexible manner, responding to their decisions at each game tick.

### Warning System

There is also a `WarningType` enum that defines various types of warnings the system may issue in response to certain player actions:
- `CustomWarning`: A general warning that could be used for any custom messages.
- `PlayerAlreadyMadeActionWarning`: A warning that indicates a player has already made their action for this tick.
- `ActionIgnoredDueToDeadWarning`: A warning that the player's action was ignored because their tank is dead.
- `SlowResponseWarning`: A warning that indicates the player's response to the game server was slow, potentially affecting their action's timing.

### Map and Related Structs

The game map is represented by several structs that define its layout, tiles, zones, and the status of various objects like tanks, bullets, walls, and more. Item with index \[0]\[0] represents top-left corner of the map. Here's a breakdown of these components:

#### **Map**
The `Map` struct represents the game world and is made up of tiles and zones:
- **tiles**: A 2D vector of `Tile`, where each tile has:
    - `zoneName`: The name of the zone for this tile (or '?' if no zone).
    - `objects` : Vector of objects in tile which can be 0 or more of:
        - `Wall`: Represents a barrier with different types:
            - `solid`: Cannot be passed through or shot through
            - `penetrable`: Cannot be passed through but can be shot through
        - `Tank`: Represents a tank on the map.
            - `ownerId`: The player owning the tank.
            - `tankType`: The type of tank (Light or Heavy)
            - `direction`: The direction the tank is facing.
            - `turret`: A `Turret` struct containing the direction of the turret and, if available, bullet information.
            - `health` (optional): Health points of the tank (absent for enemies).
            - `visibility` (optional): 2D vector of chars indicating visible tiles ('0' for invisible, '1' for visible)
            - Tank type specific fields:
                - Light tanks: `ticksToRadar`, `isUsingRadar`, `ticksToDoubleBullet`
                - Heavy tanks: `ticksToMine`, `ticksToLaser`
        - `Bullet`: Represents a bullet on the map.
            - `id`: A unique identifier for the bullet.
            - `type`: Type of bullet (basic, doubleBullet, healing, stun)
            - `speed`: The speed at which the bullet moves.
            - `direction`: The direction the bullet is traveling (up, down, left, right).
        - `Laser`: Represents a laser fired by a tank.
            - `id`: A unique identifier for the laser.
            - `orientation`: The orientation of the laser (horizontal or vertical).
        - `Mine`: Represents a mine placed by a tank.
            - `id`: A unique identifier for the mine.
            - `explosionRemainingTicks` (optional): The number of ticks remaining until the explosion finishes.

- **zones**: A vector of `Zone` structs that represent specific regions on the map.
    - **Zone**
        - `x`, `y`: The coordinates of the zone's position on the map.
        - `width`, `height`: The dimensions of the zone.
        - `name`: The character representing the zone (e.g., A, B, C).
        - `status`: A `ZoneShares` struct representing the ownership of the zone.

#### **ZoneShares**
This struct tracks the current ownership state of a zone:
- **neutral**: The percentage (0.0 to 1.0) of the zone that is neutral.
- **teamShares**: A map of team names to their ownership percentage of the zone.

---

### Team, Player, Lobby, and End Game Structs

#### **Team**
Represents a team in the game:
- **name**: The team's name.
- **color**: The team's color (as uint32_t).
- **score** (optional): The team's current score.
- **players**: A vector of `Player` structs.

#### **Player**
Represents a player in the game:
- **id**: The player's unique ID.
- **ping**: The player's ping (latency).
- **score** (optional): The player's current score.
- **ticksToRegen** (optional): The number of ticks until the player's tank regenerates.

#### **LobbyPlayer**
Represents a player in the pre-game lobby:
- **id**: The player's unique ID.
- **tankType**: The type of tank chosen by the player (Light or Heavy).

#### **LobbyTeams**
Represents a team in the pre-game lobby:
- **name**: The team's name.
- **color**: The team's color.
- **players**: A vector of `LobbyPlayer` structs.

#### **LobbyData**
Contains information about the game lobby:
- **myId**: The current player's ID.
- **teamName**: The current player's team name.
- **teams**: A vector of `LobbyTeams` structs.
- **sandboxMode**: Whether the game is in sandbox mode.
- **matchName** (optional): The name of the match.
- **gridDimension**: The size of the game grid.
- **numberOfPlayers**: The number of players in the game.
- **seed**: The random seed for the game.
- **broadcastInterval**: The interval between server broadcasts (in milliseconds).
- **eagerBroadcast**: Whether the server uses eager broadcasting.
- **version**: The server version.

#### **EndGamePlayer**
Represents a player at the end of the game:
- **id**: The player's ID.
- **kills**: The number of kills the player achieved.
- **tankType**: The type of tank the player used.

#### **EndGameTeam**
Represents a team at the end of the game:
- **name**: The team's name.
- **color**: The team's color.
- **score**: The team's final score.
- **players**: A vector of `EndGamePlayer` structs.

#### **EndGameLobby**
Contains all teams at the end of the game:
- **teams**: A vector of `EndGameTeam` structs.

---

### GameState Struct

The `GameState` struct captures the state of the game at a specific point in time:
- **time**: The current tick number (a measure of game progress).
- **teams**: A vector of `Team` structs, representing all teams in the game.
- **playerId** (optional): The ID of the current player.
- **map**: The `Map` struct representing the current state of the game world.

## Enhanced Bot Features

The updated bot implementation in `bot.cpp` includes several advanced features:

### Improved Map Visualization
- Different symbols for all object types (tanks, bullets, walls, etc.)
- Enhanced display with borders and a detailed legend
- Visual distinction between different wall types, bullet types, and tank types
- Special indicators for mines that are about to explode

### Intelligent Random Actions
- Weighted probabilities for different action types
- Tank-type specific ability selection (Light vs. Heavy)
- Support for all ability types including healing and stun bullets
- Zone capture attempts during wait actions

### Detailed Information Logging
- Turn-by-turn state reporting
- Action selection explanation
- Game initialization and ending summaries
- Warning message handling

## Running the Bot

You can run this wrapper in two different ways: locally using CLion (recommended) or Visual Studio or using Docker.

### 1. Running Locally

Have Cmake installed or install it from [here](https://cmake.org/download/)

Install Vcpkg preferably on `C:\` but you can install it anywhere
```sh
   git clone https://github.com/microsoft/vcpkg.git
   ```
then run bootstrap-vcpkg.bat on windows or .sh on linux
```sh
   cd vcpkg; .\bootstrap-vcpkg.bat
   ```

Modify line 8 or 12 in `CMakeLists.txt` depending on vcpkg location

Clone repo in chosen IDE

Preferably add x64-Release profile if it does not exist

Build through IDE or by command
```sh
   <dir to cmake.exe here> --build <dir to cmake profile here> --target HackArena2.5-StereoTanks-Cxx -j 14
   ```

### 2. Running in a Docker Container (Manual Setup)

To run the wrapper manually in a Docker container, ensure Docker is installed on
your system.

Steps:

1. Build the Docker image:
    ```sh
   cd <project dir here>
   ```
   ```sh
   docker build -t wrapper .
   ```
2. Run the Docker container:
   ```sh
   docker run --rm wrapper --teamName YourTeam --tankType light --host host.docker.internal
   ```

If the server is running on your local machine, use the
`--host host.docker.internal` flag to connect the Docker container to your local
host.

## FAQ

### What can be modified?

Anything! **Just make sure it works :)**

You can modify mentioned files and create more files in the `src/bot`
directory or `src/data` directory which will be copied to the same dir as compiled program on docker.
Modifying of any other files is not recommended, as this may prevent us from running
your bot during the competition.

Feel free to extend the functionality of the `GameState` struct or any other structs as you see fit.

### Can we include static files?

If you need to include static files that your program should access during
testing or execution, place them in the `data` folder. This folder is copied
into the Docker image and will be accessible to your application at runtime. For
example, you could include configuration files, pre-trained models, or any other
data your bot might need.

### Can we add libraries?

You can also add any libraries you want. Check vcpkg and add them to `vcpkg.json`, if not from vcpkg you can configure them yourself.

### In what format we will need to submit our bot?

You will need to submit a zip file containing the whole repository. Of course,
please, delete the cmake-build directories and any other temporary files before
submitting, so the file size is as small as possible.

### Error about missing boost?

Check `CMakeLists.txt` line 8 or 12

or

Add `-DCMAKE_TOOLCHAIN_FILE=<dir to vcpkg here>/vcpkg/scripts/buildsystems/vcpkg.cmake` to environment variables
