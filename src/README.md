# Uzak Diyarlar MUD Project

Uzak Diyarlar is an open-source Turkish MUD (Multi-User Dungeon) project. This document provides an overview of the project structure, key components, and important considerations for future development and modernization.

## Project Overview

Uzak Diyarlar is based on the MERC codebase and has been extensively modified and expanded. The project is primarily written in C and is designed to run on various Unix-like systems.

### Key Features

- Turkish language support
- Custom character creation and progression system
- Extensive world with multiple areas
- Skill and magic systems
- Player vs. Player (PvP) and Player vs. Environment (PvE) gameplay
- Faction (Cabal) system

## Project Structure

The project is organized into several key components:

### Core System Files
- `merc.h`: Main header file containing essential definitions and structures
- `db.c`, `db.h`, `db2.c`: Database management and world loading
- `comm.c`: Network communication and player input/output handling
- `interp.c`, `interp.h`: Command interpretation
- `handler.c`, `handler_room.c`: Object and character handling
- `update.c`: Game world update functions

### Action and Gameplay Files
- `act_*.c` files: Various action-related functions (movement, combat, etc.)
- `fight.c`: Combat system implementation
- `skills.c`: Skill system core functionality
- `magic.c`, `magic2.c`: Spell and magical effect implementation
- `martial_art.c`: Martial arts system

### Object and Mob Handling
- `obj_creator.c`, `obj_prog.c`: Object creation and programming
- `mob_creator.c`, `mob_prog.c`: Mobile (NPC) creation and programming
- `quest.c`: Quest system implementation
- `special.c`: Special functions for mobs and objects

### Utility and Support Files
- `ban.c`: Player banning functionality
- `lookup.c`, `tables.c`: Various lookup tables and functions
- `recycle.c`, `recycle.h`: Memory management and object recycling
- `cJSON.c`, `cJSON.h`: JSON parsing library
- `effects.c`, `flags.c`, `save.c`: Various utility functions

### Build System
- `Makefile` and `Makefile.*`: Build configuration for different platforms
- `automake`: Automake script for build system

## Key Data Structures

1. `CHAR_DATA`: Represents both player characters and NPCs
2. `OBJ_DATA`: Represents in-game objects
3. `ROOM_INDEX_DATA`: Represents individual rooms in the game world
4. `AREA_DATA`: Represents larger areas containing multiple rooms

## Important Game Systems

1. Combat System: Implemented primarily in `fight.c`, with support from various `act_*.c` files
2. Magic System: Primarily implemented in `magic.c` and `magic2.c`
3. Skill System: Core functionality in `skills.c`, with support from various other files
4. Faction (Cabal) System: Implemented across multiple files (specific files to be identified)
5. Quest System: Implemented in `quest.c`
6. Object and Mobile Programming: Implemented in `obj_prog.c` and `mob_prog.c`

## Modernization Considerations

1. Platform Support: The code currently supports multiple platforms, including some that are now deprecated. Future updates should focus on modern platforms like Linux, Windows, and macOS. Refer to `WINDOWS_SUPPORT.md` for details on Windows compatibility efforts.
2. Code Style: The project uses older C coding conventions. Updating to modern C standards could improve readability and maintainability.
3. Memory Management: The project uses custom memory management functions (`recycle.c`). Evaluating the use of modern memory management techniques could be beneficial.
4. Networking: The networking code in `comm.c` could be updated to use more modern and secure networking libraries and practices.
5. Build System: Consider modernizing the build system, potentially using CMake for better cross-platform support.

## Documentation

Uzak Diyarlar MUD project includes several documentation files to help developers understand and work with the codebase:

- `README.md`: This file, providing an overview of the project structure, key features, and important considerations.
- `STRUCTURES.md`: Detailed information about the main data structures used in the project, including their purposes and relationships.
- `GAME_SYSTEMS.md`: Comprehensive explanation of core game mechanics and systems, detailing how different components of the game interact.
- `CODING_GUIDELINES.md`: Coding conventions and guidelines specific to this project, ensuring consistency across the codebase.
- `WINDOWS_SUPPORT.md`: Documentation on Windows compatibility and porting efforts, including steps and considerations for cross-platform development.

These documentation files are designed to provide a comprehensive understanding of the project for both new and experienced developers. They should be kept up-to-date as the project evolves.

## Contributing

Contributions to Uzak Diyarlar are welcome. Please refer to the project's GitHub repository for more information on how to contribute:

[https://github.com/yelbuke/UzakDiyarlar](https://github.com/yelbuke/UzakDiyarlar)

Before contributing, please review the CODING_GUIDELINES.md file to ensure your contributions align with the project's coding standards.

## Contact

- Web: [http://www.uzakdiyarlar.net](http://www.uzakdiyarlar.net)
- Discord: [https://discord.gg/kXyZzv](https://discord.gg/kXyZzv)

## Credits

Uzak Diyarlar is managed by Jai and Maru. Special thanks to contributors: Nir, Kame, Randalin, Nyah, and Sint.

This project is based on the MERC codebase and includes contributions from various MUD codebases, including ROM and Anatolia.