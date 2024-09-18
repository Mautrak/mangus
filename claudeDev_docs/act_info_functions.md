# Key Functions in act_info.c

This document provides an overview of the main functions in act_info.c, their purposes, and how they utilize the single-letter macros for character attributes.

## do_affects_col

Purpose: Displays the magical effects currently affecting a character.

Usage of macros:
- Uses `AFF_*` macros to check for various effects on the character.
- Example: `if (IS_AFFECTED(ch, AFF_BLIND))`

## do_who_col

Purpose: Displays a list of players currently online.

Usage of macros:
- Uses `PLR_*` macros to check player flags.
- Uses `COMM_*` macros to check communication settings.
- Example: `if (IS_SET(wch->act, PLR_WANTED))`

## do_inventory

Purpose: Shows a character's inventory.

Usage of macros:
- Likely uses `ITEM_*` macros internally to handle different item types.

## do_equipment

Purpose: Displays the equipment a character is wearing or wielding.

Usage of macros:
- Likely uses `ITEM_*` and `WEAR_*` macros to handle different item types and equipment slots.

## do_compare

Purpose: Compares two items in a character's inventory.

Usage of macros:
- Likely uses `ITEM_*` macros to identify and compare item types.

## do_where

Purpose: Shows the location of characters in the same area.

Usage of macros:
- Uses `PLR_*` macros to check player flags.
- Example: `if (IS_SET(ch->act,PLR_HOLYLIGHT))`

## do_consider

Purpose: Gives an estimate of how difficult it would be to kill another character.

Usage of macros:
- Likely uses level and attribute macros internally.

## do_description

Purpose: Allows a player to set or view their character's description.

Usage of macros:
- May use `PLR_*` macros to set or check description-related flags.

## do_report

Purpose: Reports a character's current status (hit points, mana, move, experience).

Usage of macros:
- Likely uses attribute macros internally to gather status information.

## do_practice

Purpose: Allows a character to practice skills or see which skills they can practice.

Usage of macros:
- May use class and level macros to determine available skills.

## do_camp

Purpose: Allows a character to set up a camp, providing certain benefits.

Usage of macros:
- Uses `ROOM_*` macros to check if camping is allowed in the current room.
- Example: `if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))`

These functions demonstrate how the single-letter macros are used throughout the codebase to check and manipulate character attributes, room properties, and other game elements. Understanding these macros and their usage is crucial for maintaining and extending the MUD's functionality.