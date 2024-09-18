# Technical Documentation for Anatolia MUD Codebase

## Table of Contents
1. [Introduction](#introduction)
2. [Data Structures](#data-structures)
3. [Memory Management](#memory-management)
4. [File Operations](#file-operations)
5. [Random Number Generation](#random-number-generation)
6. [String Manipulation](#string-manipulation)
7. [Game Mechanics](#game-mechanics)
8. [Database Management](#database-management)
9. [Networking](#networking)
10. [Windows Compatibility Considerations](#windows-compatibility-considerations)

## Introduction
This document provides a comprehensive technical overview of the Anatolia MUD codebase. The codebase is written in C and is based on the DIKU, MERC, and ROM codebases. It includes various enhancements and custom features specific to Anatolia MUD.

## Data Structures

### AREA_DATA
Represents a game area with the following key fields:
- `name`: Name of the area
- `file_name`: File name for the area
- `vnum` range: `min_vnum` and `max_vnum`
- `age`: Area reset counter
- `nplayer`: Number of players in the area
- `reset_first`: Pointer to the first reset data
- `reset_last`: Pointer to the last reset data

### ROOM_INDEX_DATA
Represents a room in the game world:
- `vnum`: Unique room identifier
- `area`: Pointer to the AREA_DATA it belongs to
- `name`, `description`: Room details
- `exit[6]`: Array of exits (MAX_DIR = 6)
- `extra_descr`: List of extra descriptions
- `room_flags`: Bit vector of room flags
- `sector_type`: Type of terrain

### CHAR_DATA
Represents both player characters (PCs) and non-player characters (NPCs):
- `name`, `short_descr`, `long_descr`, `description`: Character descriptions
- `race`, `class`, `level`, `trust`: Character attributes
- `act`, `affected_by`: Bit vectors for flags and affects
- `position`: Current position (standing, sitting, etc.)
- `practice`: Number of practice sessions
- `hit`, `max_hit`, `mana`, `max_mana`, `move`, `max_move`: Character stats
- `gold`, `silver`: Currency
- `exp`: Experience points
- `in_room`: Current room
- `was_in_room`: Previous room (for saving state)
- `items`: Inventory and equipment

### OBJ_DATA
Represents an object in the game:
- `name`, `short_descr`, `description`: Object descriptions
- `item_type`: Type of item (weapon, armor, etc.)
- `extra_flags`, `wear_flags`: Bit vectors for object flags
- `value[5]`: Array for type-specific values
- `weight`, `cost`, `level`: Object attributes
- `timer`: Decay timer
- `affected`: List of affects on the object

## Memory Management

The codebase uses a custom memory management system to optimize allocation and deallocation of frequently used data structures. Key functions include:

- `alloc_mem(int sMem)`: Allocates memory from pre-allocated pools
- `free_mem(void *pMem, int sMem)`: Frees memory back to the pools
- `alloc_perm(int sMem)`: Allocates "permanent" memory that is never freed

Memory is managed in blocks of various sizes to reduce fragmentation and improve performance.

## File Operations

File operations are crucial for loading and saving game data. Key functions include:

- `fread_string(FILE *fp)`: Reads a string from a file
- `fread_word(FILE *fp)`: Reads a word from a file
- `fread_number(FILE *fp)`: Reads a number from a file
- `fread_letter(FILE *fp)`: Reads a single character from a file
- `fread_to_eol(FILE *fp)`: Reads and discards the rest of a line

These functions handle the specific format used in Anatolia MUD's data files.

## Random Number Generation

The codebase uses a custom random number generation system:

- `number_range(int from, int to)`: Generates a random number between 'from' and 'to'
- `number_percent()`: Generates a random percentage (1-100)
- `number_door()`: Generates a random door number (0-5)
- `number_bits(int width)`: Generates a random number with a specific bit width
- `dice(int number, int size)`: Simulates rolling dice

The `init_random_number_generator()` function is used to seed the random number generator.

## String Manipulation

Custom string manipulation functions are used throughout the codebase:

- `str_cmp(const char *astr, const char *bstr)`: Case-insensitive string comparison
- `str_prefix(const char *astr, const char *bstr)`: Checks if astr is a prefix of bstr
- `str_infix(const char *astr, const char *bstr)`: Checks if astr is contained within bstr
- `str_suffix(const char *astr, const char *bstr)`: Checks if astr is a suffix of bstr

These functions are optimized for the specific needs of the MUD codebase.

## Game Mechanics

### Combat System
The combat system is turn-based and uses various factors to determine hit chance and damage:

- `damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)`: Applies damage to a character
- `multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)`: Handles multiple attacks per round
- `one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)`: Processes a single attack

### Magic System
The magic system uses a spell table and affects:

- `say_spell(CHAR_DATA *ch, int sn)`: Handles the verbal component of spells
- `spell_<spellname>(int sn, int level, CHAR_DATA *ch, void *vo, int target)`: Individual spell functions
- `affect_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)`: Applies an affect to a character

### Skill System
Skills are handled similarly to spells but often don't use the magic system:

- `check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier)`: Handles skill improvement
- `skill_table[]`: Array containing all skill and spell data

## Database Management

The game world is loaded from and saved to various data files:

- `boot_db()`: Main function for loading the game world
- `load_area(FILE *fp)`: Loads an area file
- `load_rooms(FILE *fp)`: Loads room data
- `load_mobiles(FILE *fp)`: Loads mobile (NPC) data
- `load_objects(FILE *fp)`: Loads object data

Saving functions exist for players and certain dynamic game data.

## Networking

The codebase uses a custom networking system built on top of standard sockets:

- `game_loop_unix(int control)`: Main game loop for Unix-like systems
- `new_descriptor(int control)`: Handles new connections
- `read_from_descriptor(DESCRIPTOR_DATA *d)`: Reads input from a client
- `write_to_descriptor(int desc, char *txt, int length)`: Sends output to a client

## Windows Compatibility Considerations

To add Windows compatibility to this codebase, several areas need to be addressed:

1. **Networking**: Replace Unix-specific socket code with Winsock. This includes initializing Winsock, handling socket creation, and managing connections.

2. **File I/O**: Ensure all file paths use backslashes instead of forward slashes. Consider using a path manipulation function that works on both systems.

3. **Directory Operations**: Replace `opendir`, `readdir`, and `closedir` with Windows equivalents like `FindFirstFile`, `FindNextFile`, and `FindClose`.

4. **Time Functions**: Replace `gettimeofday` with Windows-specific time functions. Consider using a cross-platform time library.

5. **Signal Handling**: Windows doesn't support all POSIX signals. Use Windows-specific exception handling where necessary.

6. **Memory Allocation**: Ensure that memory allocation and deallocation are compatible with Windows. The custom memory management system may need adjustments.

7. **Makefiles**: Create a separate build system for Windows, possibly using Visual Studio project files or a cross-platform build system like CMake.

8. **Header Files**: Add conditional includes for Windows-specific headers (e.g., `<winsock2.h>`) and POSIX headers for Unix-like systems.

9. **Data Types**: Ensure that all data types are compatible with Windows. Pay special attention to integer sizes and pointer types.

10. **Command-line Arguments**: Modify the `main` function to use `WinMain` for Windows if creating a GUI application, or ensure `main` works correctly in a Windows console application.

11. **DLL Considerations**: If creating DLLs for Windows, ensure proper export/import declarations for functions.

12. **Threading**: If the codebase uses threading, replace POSIX threads with Windows threads or use a cross-platform threading library.

13. **Environment Variables**: Use Windows-specific functions to handle environment variables if needed.

14. **File Locking**: Replace fcntl-based file locking with Windows-specific file locking mechanisms.

15. **Case Sensitivity**: Windows filesystems are typically case-insensitive. Ensure this doesn't cause issues with file operations.

By addressing these areas, the codebase can be made compatible with Windows while maintaining its functionality on Unix-like systems. Consider using conditional compilation (`#ifdef _WIN32`) to separate Windows-specific code from Unix code.