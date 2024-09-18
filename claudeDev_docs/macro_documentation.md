# Single-Letter Macro Documentation

This document provides a comprehensive overview of the single-letter macros used in the MUD codebase, their values, and their common usage.

## Basic Macro Definitions

These macros are defined as powers of 2, allowing for efficient bitwise operations:

```c
#define A		  	1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128
#define I			256
#define J			512
#define K		        1024
#define L		 	2048
#define M			4096
#define N		 	8192
#define O			16384
#define P			32768
#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608
#define Y			16777216
#define Z			33554432
#define aa			67108864
#define bb			134217728
#define cc			268435456
#define dd			536870912
#define ee			1073741824
```

## Common Usage

These macros are used extensively throughout the codebase for various flags and bitwise operations. Here are some common applications:

### ACT Flags (for NPCs)

```c
#define ACT_IS_NPC		(A)
#define ACT_SENTINEL	    	(B)
#define ACT_SCAVENGER	      	(C)
#define ACT_AGGRESSIVE		(F)
#define ACT_STAY_AREA		(G)
#define ACT_WIMPY		(H)
#define ACT_PET			(I)
#define ACT_TRAIN		(J)
#define ACT_PRACTICE		(K)
```

### PLR Flags (for Players)

```c
#define PLR_BOUGHT_PET		(B)
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOSPLIT		(H)
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
```

### Affected By Flags

```c
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
```

### Room Flags

```c
#define ROOM_DARK		(A)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
```

### Item Flags

```c
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
```

## Usage in Code

These macros are typically used with bitwise operations. Common operations include:

1. Setting a flag:
   ```c
   SET_BIT(ch->act, ACT_AGGRESSIVE);
   ```

2. Checking if a flag is set:
   ```c
   if (IS_SET(ch->act, ACT_AGGRESSIVE))
   ```

3. Removing a flag:
   ```c
   REMOVE_BIT(ch->act, ACT_AGGRESSIVE);
   ```

4. Combining multiple flags:
   ```c
   ch->affected_by = AFF_INVISIBLE | AFF_SNEAK | AFF_HIDE;
   ```

## Important Notes

1. These macros allow for efficient storage of multiple boolean flags in a single integer.
2. Be cautious when using these macros, as they can make the code less readable if overused.
3. When adding new flags, be careful not to reuse existing values to avoid conflicts.
4. Some flags may have multiple bits set, so always use the provided macros (SET_BIT, IS_SET, REMOVE_BIT) to manipulate them rather than direct bitwise operations.

## Best Practices

1. Use the predefined constants (like ACT_AGGRESSIVE) instead of raw letters (like F) for better code readability.
2. When adding new flags, document them clearly and consider their potential interactions with existing flags.
3. Be aware of the maximum number of flags that can be stored in a single integer (typically 32 on most systems).
4. Consider using more descriptive names or enums for new feature development, reserving these single-letter macros for maintaining compatibility with existing code.

By understanding and properly using these macros, you can efficiently manipulate various attributes and states within the MUD codebase.