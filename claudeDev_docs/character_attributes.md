# Character Attribute Macros

This document provides information about single-letter macros used for character attributes in the MUD codebase, with specific examples from act_info.c.

## Common Character Attribute Macros

These macros are often used to check or set character attributes:

- `PLR_*`: Player flags
- `AFF_*`: Affect flags
- `ACT_*`: NPC action flags
- `COMM_*`: Communication flags
- `ITEM_*`: Item flags

## Examples from act_info.c

1. Player Flags (PLR_*)

```c
if (IS_SET(wch->act, PLR_WANTED))
    // ...
```

`PLR_WANTED` is likely defined as one of the single-letter macros (e.g., `J`), representing a flag for wanted players.

2. Affect Flags (AFF_*)

```c
if (IS_AFFECTED(ch, AFF_BLIND))
    // ...
if (IS_AFFECTED(ch, AFF_INVISIBLE))
    // ...
if (IS_AFFECTED(ch, AFF_DETECT_EVIL))
    // ...
```

These flags represent various effects or conditions that can be applied to a character. They are likely defined using the single-letter macros.

3. Communication Flags (COMM_*)

```c
if (IS_SET(ch->comm, COMM_COMPACT))
    // ...
if (IS_SET(ch->comm, COMM_PROMPT))
    // ...
```

These flags are used to control various communication settings for characters.

4. Room Flags

```c
if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    // ...
if (IS_SET(ch->in_room->room_flags, ROOM_PRIVATE))
    // ...
if (IS_SET(ch->in_room->room_flags, ROOM_SOLITARY))
    // ...
```

These flags represent various properties of rooms in the game world.

## Usage

These macros are typically used with the `IS_SET`, `SET_BIT`, and `REMOVE_BIT` macros to check, set, or remove flags respectively. For example:

```c
if (IS_SET(ch->act, PLR_WANTED))
    // Check if the character is wanted
SET_BIT(ch->act, PLR_WANTED)
    // Mark the character as wanted
REMOVE_BIT(ch->act, PLR_WANTED)
    // Remove the wanted status from the character
```

## Important Notes

1. The exact values of these macros are defined elsewhere in the codebase, likely in a header file.
2. Some flags may have multiple bits set, so always use the provided macros to manipulate them rather than direct bitwise operations.
3. When adding new flags, be careful not to reuse existing values, as this could lead to unexpected behavior.
4. Documentation and consistent usage of these flags across the codebase is crucial for maintainability.

Remember to refer to the specific header files or documentation for the complete list and description of each flag.