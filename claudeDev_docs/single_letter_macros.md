# Single-Letter Macros Documentation

This document provides information about single-letter macros used in the MUD codebase.

## General Macros

- `A`: 1
- `B`: 2
- `C`: 4
- `D`: 8
- `E`: 16
- `F`: 32
- `G`: 64
- `H`: 128
- `I`: 256
- `J`: 512
- `K`: 1024
- `L`: 2048
- `M`: 4096
- `N`: 8192
- `O`: 16384
- `P`: 32768
- `Q`: 65536
- `R`: 131072
- `S`: 262144
- `T`: 524288
- `U`: 1048576
- `V`: 2097152
- `W`: 4194304
- `X`: 8388608
- `Y`: 16777216
- `Z`: 33554432
- `aa`: 67108864
- `bb`: 134217728
- `cc`: 268435456
- `dd`: 536870912
- `ee`: 1073741824
- `ab`: 2147483648

These macros are used for bitwise operations and flag settings throughout the codebase. Each letter represents a power of 2, allowing for efficient storage and manipulation of multiple boolean flags in a single integer.

## Usage Examples

These macros are commonly used in various parts of the code, such as:

1. Room flags
2. Character flags
3. Object flags
4. Skill and spell properties
5. Area properties

For example, a room might have multiple flags set like this:
```c
room->room_flags = A | C | F;
```
This would set the 1st, 3rd, and 6th bits of the room_flags integer.

## Specific Examples from act_info.c

1. In the `do_affects_col` function, these macros are used to check for various effects on a character:

```c
if (IS_AFFECTED(ch, AFF_BLIND))
    // ...
if (IS_AFFECTED(ch, AFF_INVISIBLE))
    // ...
if (IS_AFFECTED(ch, AFF_DETECT_EVIL))
    // ...
```

Here, `AFF_BLIND`, `AFF_INVISIBLE`, and `AFF_DETECT_EVIL` are likely defined using these single-letter macros.

2. In the `do_who_col` function, these macros are used to check character attributes:

```c
if (IS_SET(wch->act, PLR_WANTED))
    // ...
if (IS_TRUSTED(ch,LEVEL_IMMORTAL))
    // ...
```

`PLR_WANTED` and `LEVEL_IMMORTAL` are likely defined using these macros.

3. In the `do_camp` function, room flags are checked using these macros:

```c
if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
     IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
     IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
    // ...
```

`ROOM_SAFE`, `ROOM_PRIVATE`, and `ROOM_SOLITARY` are likely defined using these macros.

## Important Note

When working with these macros, be cautious about their usage and potential conflicts. Always refer to the specific context in which they are used, as the same letter might have different meanings in different parts of the code.

It's recommended to use descriptive constants or enums instead of these single-letter macros for new code to improve readability and maintainability.