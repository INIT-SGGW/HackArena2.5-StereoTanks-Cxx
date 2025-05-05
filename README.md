# MonoTanks API wrapper in C++ for HackArena 2.5

This API wrapper for MonoTanks game for the HackArena 2.5, organized by KN init.
It is implemented as a WebSocket wrapper written in C++ programming language and
can be used to create bots for the game.

To fully test and run the game, you will also need the game server and GUI
wrapper, as the GUI provides a visual representation of gameplay. You can find
more information about the server and GUI wrapper in the following repository:

- [Server and GUI client Repository](https://github.com/INIT-SGGW/HackArena2.0-MonoTanks)

The guide to the game mechanics and tournament rules can be found on the:
- [Instruction Page](https://hackarena.pl/Assets/Game/HackArena%202.0%20-%20instrukcja.pdf)

## Development

Clone this repo using git:
```sh
git clone https://github.com/INIT-SGGW/HackArena2.5-StereoTanks-Cxx.git
```

or download the [zip file](https://github.com/INIT-SGGW/HackArena2.5-StereoTanks-Cxx/archive/refs/heads/master.zip)
and extract it.

The bot logic you are going to implement is located in `src/bot/bot.cpp` and declarations in `src/bot/bot.h`

The `Bot` class implements methods, which define the bot's
behavior. The constructor is called when the bot is created, and the
`NextMove` function is called every game tick to determine the bot's next
move. The `OnGameEnded` function is called when the game ends to provide the
final game state.

`NextMove` returns a `ResponseVariant` (`std::variant`), which can be one of the following:

- `Rotate`: This variant indicates a rotation action, allowing the player to rotate their tank and/or turret.
    - `tankRotation` (`RotationDirection`): Specifies the rotation direction of the tank (left or right).
    - `turretRotation` (`RotationDirection`): Specifies the rotation direction of the turret (left or right).

- `Move`: This variant indicates a movement action, allowing the player to move the tank.
    - `direction` (`MoveDirection`): Specifies the direction of movement (forward or backward).

- `AbilityUse`: This variant represents an ability use action, where the tank can: shoot a bullet in the direction the turret is pointing, fire double bullet, use laser, use radar, drop mine behind the tank.

- `Wait`: This variant indicates that the player chooses to wait, doing nothing during the current tick.

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
    - `isVisible`: Whether the tile is visible to the player.
    - `zoneName`: The name of the zone for this tile (or '?' if no zone).
    - `objects` : Objects in tile which can be 0 or more:
        - `Wall`: Represents an indestructible barrier.
        - `Tank`: Represents a tank on the map.
            - `ownerId`: The player owning the tank.
            - `direction`: The direction the tank is facing.
            - `turret`: A `Turret` struct containing the direction of the turret and, if available, bullet information.
            - `health` (optional): Health points of the tank (absent for enemies).
        - `Bullet`: Represents a bullet on the map.
            - `id`: A unique identifier for the bullet.
            - `speed`: The speed at which the bullet moves.
            - `direction`: The direction the bullet is traveling (up, down, left, right).
        - `Item` : Represents an item on the map. They give abilities.
          - `ItemType` : Type of item.

- **zones**: A vector of `Zone` structs that represent specific regions on the map.
    - **Zone**
        - `x`, `y`: The coordinates of the zone's position on the map.
        - `width`, `height`: The dimensions of the zone.
        - `name`: The character representing the zone (e.g., A, B, C).
        - `status`: A `ZoneStatus` struct representing the current state of the zone.

- **visibility**: A 2D vector of chars representing visibility for each tile ('0' for invisible, '1' for visible).

#### **ZoneStatus**
This struct tracks the current state of a zone, such as whether it's being captured or contested:
- **type**: A string representing the type of status (e.g., "beingCaptured", "captured", "beingContested", "beingRetaken", "neutral").
- **remainingTicks** (optional): If the zone is being captured or retaken, this field shows how many ticks are left.
- **playerId** (optional): The ID of the player capturing or that captured the zone.
- **capturedById** (optional): Used when zone is "beingContested" or "beingRetaken" instead of **playerId**.
- **retakenById** (optional): The ID of the player retaking the zone, if applicable.

---

### Player, Lobby, and End Game Structs

#### **LobbyPlayer**
Represents a player in the game lobby before the game starts:
- **id**: A unique identifier for the player.
- **nickname**: The player's chosen name.
- **color**: The player's assigned color, represented as a `uint32_t`.

#### **LobbyData**
This struct holds data related to the game lobby:
- **myId**: The ID of the current player.
- **players**: A vector of `LobbyPlayer` structs, representing all players in the lobby.
- **gridDimension**: The dimensions of the game grid.
- **numberOfPlayers**: The number of players in the game.
- **seed**: A seed value used for randomization.
- **broadcastInterval**: The interval (in milliseconds) between server broadcasts.
- **eagerBroadcast**: A boolean indicating if the game is using eager broadcasting (for faster updates).

#### **EndGamePlayer**
Similar to `LobbyPlayer`, but also includes the player's score:
- **id**: The player's ID.
- **nickname**: The player's name.
- **color**: The player's color.
- **score**: The player's final score at the end of the game.

#### **EndGameLobby**
Holds information about all players at the end of the game:
- **players**: A vector of `EndGamePlayer` structs, containing the end-game data for each player.

#### **Tank**
Represents a tank in the game, including its owner and status:
- **ownerId**: The ID of the player controlling the tank.
- **direction**: The direction the tank is facing (e.g., up, down, left, right).
- **turret**: A `Turret` struct representing the tank's turret.
- **health** (optional): The health points of the tank (absent for enemy tanks).

#### **Turret**
Represents the turret on a tank:
- **direction**: The direction the turret is facing.
- **bulletCount** (optional): The number of bullets left (absent for enemy turrets).
- **ticksToRegenBullet** (optional): The ticks remaining before a new bullet is regenerated (absent for enemies).

#### **Bullet**
Represents a bullet fired by a tank:
- **id**: A unique ID for the bullet.
- **type**: Bullet or DoubleBullet
- **speed**: The speed of the bullet.
- **direction**: The direction the bullet is traveling.

#### **Laser**
Represents a laser shot by a tank:
- **id**: A unique ID for the laser.
- **orientation**: The orientation of the laser (horizontal or vertical).

#### **Mine**
Represents a mine placed by a tank:
- **id**: A unique ID for the mine.
- **explosionRemainingTicks** (optional): The number of ticks remaining until the explosion finishes.

#### **Wall**
Represents an indestructible barrier on the game map

#### **Item**
Represents an item on the map that grants abilities to the tank:
- **type**: The type of item available.
    - `unknown`: Only when the tank is on the item.
    - `laser`: Grants the ability to shoot a laser.
    - `doubleBullet`: Allows the player to shoot two bullets simultaneously.
    - `radar`: Provides enhanced visibility of the surrounding area.
    - `mine`: Allows the player to drop a mine behind the tank.

---

### GameState Struct

The `GameState` struct captures the state of the game at a specific point in time:
- **time**: The current tick number (a measure of game progress).
- **players**: A vector of `Player` structs, representing all players currently in the game.
    - **Player**
        - **id**: The player's unique ID.
        - **nickname**: The player's name.
        - **color**: The player's assigned color.
        - **ping**: The player's ping (latency).
        - **score** (optional): The player's score (absent for enemies).
        - **ticksToRegen** (optional): The number of ticks until the player's next action can occur (e.g., reloading).
- **map**: The `Map` struct, representing the current state of the game world, including tiles, zones, and visibility.

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
   docker run --rm wrapper --nickname Cxx --host host.docker.internal
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
