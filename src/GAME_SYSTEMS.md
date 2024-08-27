# Uzak Diyarlar MUD - Game Systems Documentation

This document provides an overview of the main game systems in the Uzak Diyarlar MUD project. Understanding these systems is crucial for maintaining and expanding the game.

## Combat System

The combat system is implemented across various files, primarily in `fight.c`, with support from `act_*.c` files.

Key components:
- Initiative and turn order
- Attack rolls and damage calculation (`fight.c`)
- Defensive abilities (dodging, parrying)
- Special combat abilities and skills
- Death and resurrection mechanics

Additional files:
- `martial_art.c`: Implements martial arts combat techniques
- `skills.c`: Handles skill usage in combat

Considerations:
- The system uses a round-based approach typical of MUDs
- Various factors influence combat, including equipment, skills, and magical effects
- The system supports both Player vs. Environment (PvE) and Player vs. Player (PvP) combat

## Magic System

The magic system is primarily implemented in `magic.c` and `magic2.c`.

Key components:
- Spell definitions and effects
- Mana cost and spell level requirements
- Targeting system (self, single target, area effect)
- Magical item creation and use
- Spell resistance and saving throws

Additional files:
- `effects.c`: Handles various magical effects
- `handler.c`: Manages affect application and removal

Considerations:
- The system uses a skill-based approach for spell casting
- Spells are organized into different schools or types
- Some spells have components or requirements beyond just mana cost

## Skill System

The skill system is spread across multiple files, with core functionality in `skills.c`.

Key components:
- Skill definitions and effects
- Skill level progression
- Skill checks and success calculations
- Class-specific and general skills

Additional files:
- `class.c`: Defines class-specific skills and abilities (if present)
- `handler.c`: Manages skill learning and improvement

Considerations:
- Skills improve through use, with a chance to increase after successful use
- Some skills are passive, while others require active use
- Class restrictions apply to certain skills

## Faction (Cabal) System

The faction system, known as Cabals in Uzak Diyarlar, is implemented across multiple files.

Key components:
- Cabal definitions and membership management
- Cabal-specific abilities and restrictions
- Cabal halls and special areas
- Inter-cabal relations and conflicts

Relevant files:
- `cabal.c`: Core cabal functionality (if present)
- `act_cabal.c`: Cabal-specific actions (if present)

Considerations:
- Cabals provide a social and political dimension to the game
- Some game content may be restricted based on cabal membership
- Cabal system influences PvP interactions

## Character Progression System

Character progression is managed across various files.

Key components:
- Experience gain and level progression
- Stat increases and skill point allocation
- Class-specific advancement features
- Remort system for high-level characters

Relevant files:
- `handler.c`: Handles character data management
- `update.c`: Manages periodic updates, including experience gain
- `save.c`: Handles saving character progression

Considerations:
- The system uses a level-based progression model
- Experience can be gained through combat, quests, and other activities
- The remort system allows for continued progression beyond the level cap

## Quest System

The quest system is primarily implemented in `quest.c`.

Key components:
- Quest definitions and objectives
- Quest givers and turn-in mechanics
- Reward systems (experience, items, faction standing)
- Quest tracking and player progress

Additional files:
- `act_info.c`: Likely contains quest-related information commands
- `handler.c`: May handle quest item management

Considerations:
- Quests can be one-time or repeatable
- Some quests may be restricted by level, class, or faction
- The quest system interacts with other game systems like combat and skills

## Economy and Item System

The economy is managed through various files dealing with objects, shops, and player interactions.

Key components:
- Currency system
- Item creation, modification, and destruction
- Shop mechanics and NPC vendors
- Player-to-player trading

Relevant files:
- `obj_prog.c`: Handles object programming for special item behaviors
- `shop.c`: Implements shop functionality (if present)
- `handler.c`: Manages object manipulation and inventory
- `save.c`: Handles saving of player inventories and economy data

Considerations:
- The economy includes both in-game shops and player-driven markets
- Item properties can be influenced by crafting and magical enhancement
- Economic balance is crucial for long-term game health

## Communication System

The communication system facilitates player interaction and information dissemination.

Key components:
- Chat channels
- Private messaging
- Emotes and social actions
- Information commands

Relevant files:
- `comm.c`: Handles core communication functionality
- `act_comm.c`: Implements communication-related commands
- `act_info.c`: Provides informational commands

## World and Environment System

This system manages the game world, its areas, and environmental factors.

Key components:
- Area management
- Room interactions
- Weather and time systems
- Dynamic world events

Relevant files:
- `db.c` and `db2.c`: Handle world data loading and management
- `update.c`: Manages world updates and events
- `weather.c`: Implements weather system (if present)

## Admin and Moderation Tools

Tools for game administrators to manage the game and moderate player interactions.

Key components:
- Player management commands
- World editing tools
- Logging and monitoring systems

Relevant files:
- `act_wiz.c`: Implements administrator commands
- `ban.c`: Handles player banning functionality

## Modernization and Expansion Considerations

1. Code Organization: Consider reorganizing code to group related functionality more closely, potentially using a more modular approach.

2. Config-Driven Design: Evaluate moving more game parameters into configuration files for easier balancing and modification. Consider leveraging the JSON parsing capabilities (cJSON.c and cJSON.h) for configuration management.

3. Scripting Integration: Consider integrating a scripting language for easier creation of quests, NPCs, and dynamic content. The existing mob and object program systems (mob_prog.c and obj_prog.c) could be expanded or replaced with a more flexible scripting system.

4. Performance Optimization: Review CPU-intensive systems like combat for potential optimizations, especially for handling larger numbers of concurrent players. The update.c file might be a good starting point for optimization efforts.

5. Extensibility: Plan for how new features or game mechanics can be integrated into existing systems with minimal disruption. The modular nature of the existing systems provides a good foundation for extensibility.

6. Cross-Platform Compatibility: As outlined in WINDOWS_SUPPORT.md, consider the implications of making the game more portable across different operating systems.

7. Security Enhancements: Review and update security measures, especially in network communication (comm.c) and user input handling.

8. Database Integration: Consider integrating a database system for more efficient data management and querying capabilities, especially for complex systems like the economy and quests.

Understanding these game systems and their interactions is crucial for maintaining and expanding the Uzak Diyarlar MUD. Each system presents unique challenges and opportunities for enhancement and modernization. Always refer to the actual implementation in the source files for the most up-to-date and detailed information about each system.