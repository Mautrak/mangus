import sys
import os
import argparse
import io

# --- Configuration ---
# Value from db.c - used for simulated overflow check
MAX_STRING_BUFFER = 3500000
# Common encoding for older MUD files, adjust if needed
FILE_ENCODING = 'UTF-8'
VERBOSE = False

# --- Global State (Simulated) ---
# Simulates the single global buffer in the C code
current_buffer_usage = 0
# Tracks the current file being processed
current_filename = ""
# Tracks the current line number
current_line = 1

# --- Custom Exception ---
class AreaParseException(Exception):
    def __init__(self, message, filename=None, line=None):
        self.filename = filename or current_filename
        self.line = line or current_line
        full_message = f"Validation Error: {self.filename}:{self.line}: {message}"
        super().__init__(full_message)

# --- Character Reading Helper ---
# Simulates getc() and tracks line numbers
def read_char_sim(f):
    global current_line
    char = f.read(1)
    if char == '\n':
        current_line += 1
    # We don't return EOF explicitly, empty string signifies it
    return char

# --- C Function Simulators ---

# Simulates isspace() - includes space, \t, \n, \v, \f, \r
def is_space_sim(char):
    return char in ' \t\n\v\f\r'

# Simulates fread_letter: Reads and returns the first non-whitespace character
def read_letter_sim(f):
    while True:
        char = read_char_sim(f)
        if not char: # EOF
            return None
        if not is_space_sim(char):
            return char

# Simulates skipping whitespace used by many fread_ functions
def skip_whitespace_sim(f):
    pos = f.tell()
    char = read_char_sim(f)
    while char and is_space_sim(char):
        pos = f.tell()
        char = read_char_sim(f)
    if char: # Put back the non-whitespace char
        f.seek(pos)
    else: # EOF reached
        pass

# Simulates fread_to_eol: Reads until \n or \r, then consumes trailing \n/\r
def read_to_eol_sim(f):
    try:
        # Read until the first newline character
        while True:
            char = read_char_sim(f)
            if not char or char in '\n\r':
                break
        # Consume any immediate following newline characters
        while True:
            pos = f.tell()
            char = read_char_sim(f)
            if not char or char not in '\n\r':
                if char: # Put back the character that wasn't \n or \r
                   f.seek(pos)
                break
    except Exception as e:
        raise AreaParseException(f"Error during read_to_eol_sim: {e}")

# Simulates fread_number: Reads an integer, handling signs and |
def read_number_sim(f, context="number"):
    skip_whitespace_sim(f)
    pos = f.tell()
    sign = 1
    start_char = read_char_sim(f)

    if not start_char:
         raise AreaParseException(f"EOF found while expecting {context}.")

    if start_char == '+':
        pass # positive sign
    elif start_char == '-':
        sign = -1
    elif start_char.isdigit():
        f.seek(pos) # Put back the digit
    else:
        raise AreaParseException(f"Expected digit or sign for {context}, found '{start_char}'")

    number_str = ""
    while True:
        char = read_char_sim(f)
        if not char: # EOF is acceptable after digits
            break
        if char.isdigit():
            number_str += char
        else:
            f.seek(f.tell() - 1) # Put back non-digit
            break

    if not number_str:
        raise AreaParseException(f"No digits found for {context} after sign (if any).")

    value = int(number_str) * sign

    # Check for '|' (used for combining numbers/flags in Merc)
    pos = f.tell()
    char = read_char_sim(f)
    if char == '|':
        try:
            value |= read_number_sim(f, context + " part after |")
        except AreaParseException as e:
            raise AreaParseException(f"Error parsing part after '|' for {context}: {e}")
    elif char:
        f.seek(pos) # Put back if not '|'

    return value

# Simulates fread_flag: Reads numerical or A-Za-z flags, handles |
# Note: Exact bit value conversion (flag_convert) isn't needed for validation,
# just parsing the format correctly.
def read_flag_sim(f, context="flag"):
    skip_whitespace_sim(f)
    pos = f.tell()
    start_char = read_char_sim(f)

    if not start_char:
         raise AreaParseException(f"EOF found while expecting {context}.")

    # Check if it looks like a number (potentially negative)
    is_potentially_numeric = start_char.isdigit() or start_char == '-'
    f.seek(pos) # Put back the character to let read_number handle sign

    flag_value = 0
    if is_potentially_numeric:
        try:
            # Try reading as a number first
            flag_value = read_number_sim(f, context)
            return flag_value # Successfully read as number
        except AreaParseException:
             # If it failed as a number, it might be letters, reset position
             f.seek(pos)
             pass # Continue to try reading as letters

    # Read as letters (A-Za-z)
    flag_str = ""
    read_letters = False
    while True:
        char = read_char_sim(f)
        if not char:
            break
        if char.isalpha():
            flag_str += char
            read_letters = True
        else:
            f.seek(f.tell() - 1) # Put back non-alpha
            break

    if read_letters:
         # We don't calculate the actual flag value, just consume the letters
         pass # Successfully read letters
    elif not is_potentially_numeric: # If it wasn't numeric and no letters were read
        raise AreaParseException(f"Expected digits or letters for {context}, starting with '{start_char}'")

    # Check for '|' combining flags
    pos = f.tell()
    char = read_char_sim(f)
    if char == '|':
        try:
            # Recursively read the next part
            read_flag_sim(f, context + " part after |")
        except AreaParseException as e:
            raise AreaParseException(f"Error parsing part after '|' for {context}: {e}")
    elif char:
        f.seek(pos)

    # For validation, we just need to know we parsed *something* validly.
    # Return 1 if letters were read, or the numeric value if that was read.
    # This isn't perfect but reflects that *some* flag value was consumed.
    return flag_value if is_potentially_numeric else 1 if read_letters else 0


# Simulates fread_word: Reads a single word, handling quotes
def read_word_sim(f, context="word"):
    skip_whitespace_sim(f)
    start_pos = f.tell()
    first_char = read_char_sim(f)

    if not first_char:
        raise AreaParseException(f"EOF found when expecting {context}.")

    word = ""
    end_char = ' ' # Default delimiter is whitespace

    if first_char == "'" or first_char == '"':
        end_char = first_char # Delimiter is the quote itself
    else:
        word += first_char # Add the first char if it wasn't a quote

    # Read until delimiter or EOF
    while True:
        char = read_char_sim(f)
        if not char:
            if end_char != ' ': # Unterminated quote
                 raise AreaParseException(f"EOF found before closing quote ({end_char}) for {context}.")
            break # End of file is okay for unquoted words

        if end_char == ' ':
            if is_space_sim(char):
                f.seek(f.tell() - 1) # Put back the whitespace
                break
            else:
                word += char
        else: # Inside quotes
            if char == end_char:
                break # Found closing quote
            else:
                word += char

    if not word and end_char == ' ':
         # This can happen if only whitespace was read after skip_whitespace_sim
         # which suggests an issue or just empty space before next token
         # Let's re-raise maybe? Or signal empty word? For now, raise.
         f.seek(start_pos) # Go back to where we started
         raise AreaParseException(f"Could not read a valid {context}. Found '{first_char}' after skipping whitespace.")


    return word

# *** THE CORE SIMULATOR ***
# Simulates fread_string: Reads until '~', handles \n\r, checks buffer
def read_string_sim(f, context="string"):
    global current_buffer_usage

    start_line = current_line
    skip_whitespace_sim(f)
    first_char_pos = f.tell()
    first_char = read_char_sim(f)

    if not first_char:
        raise AreaParseException(f"EOF found when expecting start of {context}.")

    # Handle the special case: first non-whitespace is '~' -> empty string
    if first_char == '~':
        # Consumes 0 bytes from the simulated C buffer perspective
        return "" # Represents the empty string pointer in C

    # Put the first character back to be processed by the main loop
    f.seek(first_char_pos)

    str_buffer = io.StringIO()
    bytes_added = 0

    while True:
        char = read_char_sim(f)

        # Check 1: EOF before '~'
        if not char:
            raise AreaParseException(f"EOF hit before finding terminating '~' for {context} starting near line {start_line}. String so far: '{str_buffer.getvalue()[-100:]}'") # Show last part

        # Check 2: Termination character '~'
        if char == '~':
            break # End of string

        # Check 3: Buffer Overflow (Simulated)
        # Estimate bytes: 1 for char, +1 if \n becomes \n\r
        potential_add = 1
        if char == '\n':
            potential_add = 2

        if current_buffer_usage + bytes_added + potential_add > MAX_STRING_BUFFER:
             string_so_far = str_buffer.getvalue()
             raise AreaParseException(f"Simulated MAX_STRING ({MAX_STRING_BUFFER}) exceeded while reading {context} starting near line {start_line}. Processed {current_buffer_usage + bytes_added} bytes. String so far: '{string_so_far[-100:]}'")

        # Process character based on C logic
        if char == '\n':
            str_buffer.write('\n')
            str_buffer.write('\r')
            bytes_added += 2
        elif char == '\r':
            pass # Ignored by fread_string
        else:
            str_buffer.write(char)
            bytes_added += 1

    # Successfully read the string
    result_string = str_buffer.getvalue()
    current_buffer_usage += bytes_added # Update global buffer usage
    if VERBOSE:
         print(f"Read string ({context}): {bytes_added} bytes. Total buffer: {current_buffer_usage}. String: '{result_string[:50]}...'")
    return result_string


# --- Area File Section Parsers ---

# These functions mimic the load_ functions in db.c/db2.c
# They must call the read_*_sim functions in the correct order

def parse_areadata(f):
    if VERBOSE: print(f"Parsing #AREADATA near line {current_line}")
    # Based on load_areadata in db.c
    allowed_keys = {"BUILDER", "END", "FILENAME", "HIGHLEVEL", "LANGUAGE",
                    "LOWLEVEL", "MINVNUM", "MAXVNUM", "NAME", "PATH",
                    "TRANSLATOR", "YONELIM_IYI", "YONELIM_YANSIZ", "YONELIM_KEM"}
    while True:
        pos = f.tell()
        word = read_word_sim(f, "AREADATA keyword").upper()
        if word == "END":
            break
        if word not in allowed_keys:
             raise AreaParseException(f"Unknown keyword '{word}' in #AREADATA section.")

        # Determine if the value should be a string or number based on keyword
        if word in ["BUILDER", "FILENAME", "LANGUAGE", "NAME", "PATH", "TRANSLATOR"]:
            read_string_sim(f, f"AREADATA {word} value")
        elif word in ["HIGHLEVEL", "LOWLEVEL", "MINVNUM", "MAXVNUM", "YONELIM_IYI", "YONELIM_YANSIZ", "YONELIM_KEM"]:
            read_number_sim(f, f"AREADATA {word} value")
        else:
             # Should not happen due to check above, but safety first
             read_to_eol_sim(f) # Skip unexpected format

    if VERBOSE: print(f"Finished #AREADATA near line {current_line}")

def parse_helps(f):
    if VERBOSE: print(f"Parsing #HELPS near line {current_line}")
    # Based on load_helps
    while True:
        pos = f.tell()
        level = read_number_sim(f, "Help level")
        keyword = read_string_sim(f, "Help keyword")
        if keyword == '$':
            # Need to ensure '$' was the *only* content after level
            # read_string_sim handles the '~' termination
            read_string_sim(f, "Help text after $ keyword") # Read the final empty text string
            break
        read_string_sim(f, f"Help text for '{keyword}'")
    if VERBOSE: print(f"Finished #HELPS near line {current_line}")

def parse_mobiles_new(f):
    if VERBOSE: print(f"Parsing #NEW_MOBILES near line {current_line}")
    # Based on load_new_mobiles in db2.c
    while True:
        pos = f.tell()
        letter = read_letter_sim(f)
        if letter is None: raise AreaParseException("EOF found expecting '#' for mobile record.")
        if letter != '#': raise AreaParseException(f"Expected '#' for mobile record, found '{letter}'.")

        vnum = read_number_sim(f, "Mobile Vnum")
        if vnum == 0:
            break # End of mobiles section

        read_string_sim(f, f"Mobile {vnum} Name")
        read_string_sim(f, f"Mobile {vnum} Short Desc")
        read_string_sim(f, f"Mobile {vnum} Long Desc")
        read_string_sim(f, f"Mobile {vnum} Description")
        read_string_sim(f, f"Mobile {vnum} Race Name") # Reads race name string
        read_flag_sim(f, f"Mobile {vnum} Act Flags")
        read_flag_sim(f, f"Mobile {vnum} Affect Flags")
        read_number_sim(f, f"Mobile {vnum} Level")
        # The C code doesn't read more fields directly here in the main loop
        # It derives alignment, stats etc later.
        # We need to ensure the line is consumed correctly implicitly
        # This might be tricky if there are optional fields not shown?
        # Assuming the format is fixed as read here.
        # Read the rest of the line implicitly? No, C code doesn't read_to_eol here.
        # Let's assume the flags/level are the last things read for the main block.

    if VERBOSE: print(f"Finished #NEW_MOBILES near line {current_line}")

def parse_objects_old(f, keyword="#OBJOLD"):
    if VERBOSE: print(f"Parsing {keyword} near line {current_line}")
    # Based on load_old_obj / load_new_old_obj in db.c
    while True:
        pos = f.tell()
        letter = read_letter_sim(f)
        if letter is None: raise AreaParseException(f"EOF found expecting '#' for {keyword} record.")
        if letter != '#': raise AreaParseException(f"Expected '#' for {keyword} record, found '{letter}'.")

        vnum = read_number_sim(f, f"{keyword} Vnum")
        if vnum == 0:
            break # End of objects section

        read_string_sim(f, f"{keyword} {vnum} Name")
        read_string_sim(f, f"{keyword} {vnum} Short Desc")
        read_string_sim(f, f"{keyword} {vnum} Description")
        read_string_sim(f, f"{keyword} {vnum} Action Desc (Unused?)") # Consumed but unused in C

        read_number_sim(f, f"{keyword} {vnum} Item Type")
        read_flag_sim(f, f"{keyword} {vnum} Extra Flags")
        read_flag_sim(f, f"{keyword} {vnum} Wear Flags")
        read_number_sim(f, f"{keyword} {vnum} Value 0")
        read_number_sim(f, f"{keyword} {vnum} Value 1")
        read_number_sim(f, f"{keyword} {vnum} Value 2")
        read_number_sim(f, f"{keyword} {vnum} Value 3")
        # Value 4 is set to 0 in C, not read
        # Level is set to 0 in C, not read
        # Condition is set to 100 in C, not read
        read_number_sim(f, f"{keyword} {vnum} Weight")
        read_number_sim(f, f"{keyword} {vnum} Cost")
        read_number_sim(f, f"{keyword} {vnum} Cost Per Day (Unused?)")

        # Parse E/A blocks
        while True:
            e_a_pos = f.tell()
            e_a_letter = read_letter_sim(f)
            if e_a_letter == 'E':
                read_string_sim(f, f"{keyword} {vnum} Extra Desc Keyword")
                read_string_sim(f, f"{keyword} {vnum} Extra Desc Text")
            elif e_a_letter == 'A':
                read_number_sim(f, f"{keyword} {vnum} Affect Location")
                read_number_sim(f, f"{keyword} {vnum} Affect Modifier")
            else:
                if e_a_letter: # Put back if not E or A
                    f.seek(e_a_pos)
                break # End of E/A blocks for this object

    if VERBOSE: print(f"Finished {keyword} near line {current_line}")

def parse_objects_new(f, keyword="#OBJECTS"):
    if VERBOSE: print(f"Parsing {keyword} near line {current_line}")
    # Based on load_objects / load_new_objects in db2.c
    while True:
        pos = f.tell()
        letter = read_letter_sim(f)
        if letter is None: raise AreaParseException(f"EOF found expecting '#' for {keyword} record.")
        if letter != '#': raise AreaParseException(f"Expected '#' for {keyword} record, found '{letter}'.")

        vnum = read_number_sim(f, f"{keyword} Vnum")
        if vnum == 0: break

        read_string_sim(f, f"{keyword} {vnum} Name")
        read_string_sim(f, f"{keyword} {vnum} Short Desc")
        read_string_sim(f, f"{keyword} {vnum} Description")
        read_string_sim(f, f"{keyword} {vnum} Material")
        item_type_str = read_word_sim(f, f"{keyword} {vnum} Item Type")
        # We don't need the exact type, just parse correctly
        read_flag_sim(f, f"{keyword} {vnum} Extra Flags")
        read_flag_sim(f, f"{keyword} {vnum} Wear Flags")

        # Values depend on item type, read based on C logic structure
        # The exact lookup isn't needed, just the *number* of reads
        # Weapon: word num num word flag
        # Container: num flag num num num
        # Drink/Fountain: num num word num num
        # Wand/Staff: num num num word num
        # Potion/Pill/Scroll: num word word word word
        # Default: flag flag flag flag flag
        # We will simplify and assume 5 reads, potentially mixed types,
        # which should cover most cases for validation. A more precise
        # parser could use the item_type_str. This is a tradeoff.
        # Let's try to be a bit more specific based on common types.
        if item_type_str.lower() in ["weapon"]:
             read_word_sim(f, f"{keyword} {vnum} Value 0 (Weapon Type)")
             read_number_sim(f, f"{keyword} {vnum} Value 1")
             read_number_sim(f, f"{keyword} {vnum} Value 2")
             read_word_sim(f, f"{keyword} {vnum} Value 3 (Damage Type)")
             read_flag_sim(f, f"{keyword} {vnum} Value 4")
        elif item_type_str.lower() in ["container"]:
             read_number_sim(f, f"{keyword} {vnum} Value 0")
             read_flag_sim(f, f"{keyword} {vnum} Value 1")
             read_number_sim(f, f"{keyword} {vnum} Value 2")
             read_number_sim(f, f"{keyword} {vnum} Value 3")
             read_number_sim(f, f"{keyword} {vnum} Value 4")
        elif item_type_str.lower() in ["drink", "fountain"]:
             read_number_sim(f, f"{keyword} {vnum} Value 0")
             read_number_sim(f, f"{keyword} {vnum} Value 1")
             read_word_sim(f, f"{keyword} {vnum} Value 2 (Liq Type)")
             read_number_sim(f, f"{keyword} {vnum} Value 3")
             read_number_sim(f, f"{keyword} {vnum} Value 4")
        elif item_type_str.lower() in ["wand", "staff"]:
             read_number_sim(f, f"{keyword} {vnum} Value 0")
             read_number_sim(f, f"{keyword} {vnum} Value 1")
             read_number_sim(f, f"{keyword} {vnum} Value 2")
             read_word_sim(f, f"{keyword} {vnum} Value 3 (Spell)")
             read_number_sim(f, f"{keyword} {vnum} Value 4")
        elif item_type_str.lower() in ["potion", "pill", "scroll"]:
             read_number_sim(f, f"{keyword} {vnum} Value 0")
             read_word_sim(f, f"{keyword} {vnum} Value 1 (Spell 1)")
             read_word_sim(f, f"{keyword} {vnum} Value 2 (Spell 2)")
             read_word_sim(f, f"{keyword} {vnum} Value 3 (Spell 3)")
             read_word_sim(f, f"{keyword} {vnum} Value 4 (Spell 4)")
        else: # Default - 5 flags
             read_flag_sim(f, f"{keyword} {vnum} Value 0")
             read_flag_sim(f, f"{keyword} {vnum} Value 1")
             read_flag_sim(f, f"{keyword} {vnum} Value 2")
             read_flag_sim(f, f"{keyword} {vnum} Value 3")
             read_flag_sim(f, f"{keyword} {vnum} Value 4")

        read_number_sim(f, f"{keyword} {vnum} Level")
        read_number_sim(f, f"{keyword} {vnum} Weight")
        read_number_sim(f, f"{keyword} {vnum} Cost")
        cond_letter = read_letter_sim(f) # Condition P/G/A/W/D/B/R
        if cond_letter not in 'PGAWDBR':
            raise AreaParseException(f"Invalid condition letter '{cond_letter}' for {keyword} {vnum}.")

        # Parse E/A/F blocks
        while True:
            e_a_f_pos = f.tell()
            e_a_f_letter = read_letter_sim(f)
            if e_a_f_letter == 'E':
                read_string_sim(f, f"{keyword} {vnum} Extra Desc Keyword")
                read_string_sim(f, f"{keyword} {vnum} Extra Desc Text")
            elif e_a_f_letter == 'A':
                read_number_sim(f, f"{keyword} {vnum} Affect Location")
                read_number_sim(f, f"{keyword} {vnum} Affect Modifier")
            elif e_a_f_letter == 'F':
                read_letter_sim(f) # where A/I/R/V/D
                read_number_sim(f, f"{keyword} {vnum} Affect 'F' Location")
                read_number_sim(f, f"{keyword} {vnum} Affect 'F' Modifier")
                read_flag_sim(f, f"{keyword} {vnum} Affect 'F' Bitvector")
            # --- Modification for NEW_OBJECTS ---
            # The C code has a bug/difference: it *always* reads the numbers for 'A' and 'F'
            # even inside the NEW_OBJECTS loader where it doesn't store them.
            # We need to replicate this consumption.
            elif e_a_f_letter is not None and keyword == "#NEW_OBJECTS":
                if e_a_f_letter == 'A':
                     # Consume the numbers even though not used by this loader variant
                     read_number_sim(f, f"{keyword} {vnum} (Unused) Affect Location")
                     read_number_sim(f, f"{keyword} {vnum} (Unused) Affect Modifier")
                elif e_a_f_letter == 'F':
                     # Consume the type letter and numbers/flag
                     read_letter_sim(f) # where A/I/R/V/D
                     read_number_sim(f, f"{keyword} {vnum} (Unused) Affect 'F' Location")
                     read_number_sim(f, f"{keyword} {vnum} (Unused) Affect 'F' Modifier")
                     read_flag_sim(f, f"{keyword} {vnum} (Unused) Affect 'F' Bitvector")
                else:
                    # Put back if not E/A/F
                    f.seek(e_a_f_pos)
                    break
            # --- End Modification ---
            else:
                if e_a_f_letter: # Put back if not E/A/F
                    f.seek(e_a_f_pos)
                break # End of E/A/F blocks for this object

    if VERBOSE: print(f"Finished {keyword} near line {current_line}")


def parse_resets(f):
    if VERBOSE: print(f"Parsing #RESETS near line {current_line}")
    # Based on load_resets in db.c
    last_reset_was_mob = False # Track for G/E resets
    while True:
        pos = f.tell()
        letter = read_letter_sim(f)
        if letter is None: raise AreaParseException("EOF found expecting Reset command letter.")
        if letter == 'S':
            break # End of resets section
        if letter == '*':
            read_to_eol_sim(f) # Skip comment line
            continue

        # Read the common fields (command already read)
        read_number_sim(f, "Reset if_flag (Unused)") # arg0 - unused
        read_number_sim(f, f"Reset {letter} arg1")  # vnum or room vnum
        read_number_sim(f, f"Reset {letter} arg2")  # limit or door/exit num
        arg3 = 0
        arg4 = 0

        if letter in 'MOR': # These have arg3 (vnum or state)
           arg3 = read_number_sim(f, f"Reset {letter} arg3")
        if letter in 'MP': # These have arg4 (limit)
           arg4 = read_number_sim(f, f"Reset {letter} arg4")

        read_to_eol_sim(f) # Consume rest of the line

        # Track if last reset loaded a mobile
        last_reset_was_mob = (letter == 'M')

        # Basic validation based on C code checks
        if letter == 'D': # Door state
             if arg3 < 0 or arg3 > 2:
                  print(f"Warning: {current_filename}:{current_line}: Reset D: Invalid lock state {arg3} (expected 0-2).")
        elif letter == 'R': # Randomize exits
             if arg2 < 0 or arg2 > 6: # Max exits + 1 ? ROM uses 6, Diku might use 5? Check C code.
                 print(f"Warning: {current_filename}:{current_line}: Reset R: Invalid exit count {arg2} (expected 0-6?).")
        elif letter in 'GE': # Give/Equip
             if not last_reset_was_mob:
                  # This isn't strictly enforced by C exit(), but is bad practice
                  print(f"Warning: {current_filename}:{current_line}: Reset {letter}: G/E command does not immediately follow an M command.")
        # Further validation could involve checking if VNUMs exist, but that needs
        # storing indices read earlier, adding complexity.

    if VERBOSE: print(f"Finished #RESETS near line {current_line}")

def parse_rooms(f):
    if VERBOSE: print(f"Parsing #ROOMS near line {current_line}")
    # Based on load_rooms in db.c
    while True:
        pos = f.tell()
        letter = read_letter_sim(f)
        if letter is None: raise AreaParseException("EOF found expecting '#' for room record.")
        if letter != '#': raise AreaParseException(f"Expected '#' for room record, found '{letter}'.")

        vnum = read_number_sim(f, "Room Vnum")
        if vnum == 0: break # End of rooms section

        read_string_sim(f, f"Room {vnum} Name")
        read_string_sim(f, f"Room {vnum} Description")
        read_number_sim(f, f"Room {vnum} Area Num (Unused?)")
        read_flag_sim(f, f"Room {vnum} Room Flags")
        read_number_sim(f, f"Room {vnum} Sector Type")

        # Parse D/E/H/M/O/S blocks
        while True:
            block_pos = f.tell()
            block_letter = read_letter_sim(f)
            if block_letter == 'S': # End of current room
                break
            elif block_letter == 'D': # Exit
                door = read_number_sim(f, f"Room {vnum} Exit Door (0-5)")
                if door < 0 or door > 5: # Diku/Merc standard
                    print(f"Warning: {current_filename}:{current_line}: Room {vnum}: Invalid exit door number {door}.")
                read_string_sim(f, f"Room {vnum} Exit D{door} Desc")
                read_string_sim(f, f"Room {vnum} Exit D{door} Keyword")
                read_number_sim(f, f"Room {vnum} Exit D{door} Locks")
                read_number_sim(f, f"Room {vnum} Exit D{door} Key Vnum")
                read_number_sim(f, f"Room {vnum} Exit D{door} To Vnum")
            elif block_letter == 'E': # Extra Description
                read_string_sim(f, f"Room {vnum} Extra Desc Keyword")
                read_string_sim(f, f"Room {vnum} Extra Desc Text")
            elif block_letter == 'H': # Heal rate
                 read_number_sim(f, f"Room {vnum} Heal Rate")
            elif block_letter == 'M': # Mana rate
                 read_number_sim(f, f"Room {vnum} Mana Rate")
            elif block_letter == 'O': # Owner
                 read_string_sim(f, f"Room {vnum} Owner")
            else:
                 if block_letter:
                      raise AreaParseException(f"Unexpected block letter '{block_letter}' in Room {vnum} definition.")
                 else: # EOF
                      raise AreaParseException(f"EOF encountered within Room {vnum} definition.")
    if VERBOSE: print(f"Finished #ROOMS near line {current_line}")

def parse_shops(f):
    if VERBOSE: print(f"Parsing #SHOPS near line {current_line}")
    # Based on load_shops in db.c
    while True:
        pos = f.tell()
        # Must read keeper first to check for 0
        keeper_vnum = read_number_sim(f, "Shop Keeper Vnum")
        if keeper_vnum == 0:
            break # End of shops section

        # Read trade types (MAX_TRADE = 5 in merc.h)
        for i in range(5):
            read_number_sim(f, f"Shop {keeper_vnum} Buy Type {i+1}")
        read_number_sim(f, f"Shop {keeper_vnum} Profit Buy")
        read_number_sim(f, f"Shop {keeper_vnum} Profit Sell")
        read_number_sim(f, f"Shop {keeper_vnum} Open Hour")
        read_number_sim(f, f"Shop {keeper_vnum} Close Hour")
        read_to_eol_sim(f) # Consume rest of line
    if VERBOSE: print(f"Finished #SHOPS near line {current_line}")

def parse_socials(f):
    if VERBOSE: print(f"Parsing #SOCIALS near line {current_line}")
    # Based on load_socials in db2.c
    while True:
        pos = f.tell()
        name = read_word_sim(f, "Social Name")
        if name == '#0':
             break # End of socials

        read_to_eol_sim(f) # Eat rest of name line

        fields = [
            "char_no_arg", "others_no_arg", "char_found", "others_found",
            "vict_found", "char_not_found", "char_auto", "others_auto"
        ]
        for field_name in fields:
            field_pos = f.tell()
            # load_socials uses fread_string_eol, which we don't have simulated
            # Let's assume it's equivalent to read_string_sim for validation
            # It reads until \n or \r, not '~'. This is a key difference!
            # We need a dedicated simulator or adapt read_string_sim
            # Let's try read_to_eol_sim and check if it looks like a string start
            # This is imprecise. The BEST way is to implement fread_string_eol_sim
            # For now, we'll use read_string_sim which will likely FAIL if socials
            # are not '~' terminated, highlighting the C code vs file format mismatch.
            # Or maybe socials ARE '~' terminated in this codebase despite db2.c? Check .are files.
            # Assuming standard '~' termination for now based on other sections.
            # If validation fails here, review load_socials and .are files closely.
            try:
                text = read_string_sim(f, f"Social {name}: {field_name}")
                if text == '$': # Check if it was just the '$' sign read as a "string"
                     # This check is weak because read_string_sim expects '~'
                     # If the file *actually* has '$' followed by newline,
                     # read_string_sim would fail before this check.
                     # Assuming '$' means null in the C logic.
                     pass
            except AreaParseException as e:
                 # Maybe it *was* just '$' followed by newline? Let's check.
                 f.seek(field_pos)
                 potential_dollar = read_letter_sim(f)
                 if potential_dollar == '$':
                      # Okay, it was a '$', consume it and the EOL
                      read_to_eol_sim(f)
                 elif potential_dollar == '#':
                      # It's the start of the next social
                      f.seek(field_pos) # Put back the '#'
                      break # Move to next social
                 else:
                      # It was a genuine error reading the string
                      raise e # Re-raise the original exception

            # Check if we hit the next social marker early
            next_social_pos = f.tell()
            potential_hash = read_letter_sim(f)
            if potential_hash == '#':
                f.seek(next_social_pos) # Put back '#'
                break # Start next social
            elif potential_hash:
                f.seek(next_social_pos) # Put back whatever was read
            else: # EOF
                raise AreaParseException(f"EOF encountered in middle of social '{name}'")

    if VERBOSE: print(f"Finished #SOCIALS near line {current_line}")

def parse_specials(f):
    if VERBOSE: print(f"Parsing #SPECIALS near line {current_line}")
    # Based on load_specials in db.c
    while True:
        pos = f.tell()
        letter = read_letter_sim(f)
        if letter is None: raise AreaParseException("EOF found expecting Special command letter.")
        if letter == 'S':
            break # End of specials section
        if letter == '*':
            read_to_eol_sim(f) # Skip comment line
            continue

        if letter == 'M':
            read_number_sim(f, "Special Mob Vnum")
            read_word_sim(f, "Special Func Name")
        else:
            raise AreaParseException(f"Unexpected command letter '{letter}' in #SPECIALS (expected M, S, or *).")

        read_to_eol_sim(f) # Consume rest of line
    if VERBOSE: print(f"Finished #SPECIALS near line {current_line}")

# --- TODO: Add parsers for missing sections if needed ---
# #OMPROGS, #OLIMITS, #PRACTICERS, #RESETMESSAGE, #FLAG
# These follow similar patterns: read letter/keyword, read args (nums/words/strings), consume EOL.

# --- Main Validator ---

def validate_area_file(filepath):
    global current_buffer_usage, current_filename, current_line
    current_buffer_usage = 0
    current_filename = filepath
    current_line = 1
    errors_found = 0
    warnings_found = 0 # Use sparingly for non-fatal issues

    print(f"--- Validating {filepath} ---")

    # Define parsers for known sections
    section_parsers = {
        "AREADATA": parse_areadata,
        "HELPS": parse_helps,
        "NEW_MOBILES": parse_mobiles_new,
        "OBJOLD": lambda f: parse_objects_old(f, "#OBJOLD"),
        "NEW_OBJOLD": lambda f: parse_objects_old(f, "#NEW_OBJOLD"),
        "OBJECTS": lambda f: parse_objects_new(f, "#OBJECTS"),
        "NEW_OBJECTS": lambda f: parse_objects_new(f, "#NEW_OBJECTS"),
        "RESETS": parse_resets,
        "ROOMS": parse_rooms,
        "SHOPS": parse_shops,
        "SOCIALS": parse_socials,
        "SPECIALS": parse_specials,
        # Add other section parsers here as implemented
        "OMPROGS": lambda f: print(f"Warning: Skipping unimplemented section #OMPROGS near line {current_line}"), # Example skip
        "OLIMITS": lambda f: print(f"Warning: Skipping unimplemented section #OLIMITS near line {current_line}"),
        "PRACTICERS": lambda f: print(f"Warning: Skipping unimplemented section #PRACTICERS near line {current_line}"),
        "RESETMESSAGE": lambda f: print(f"Warning: Skipping unimplemented section #RESETMESSAGE near line {current_line}"),
        "FLAG": lambda f: print(f"Warning: Skipping unimplemented section #FLAG near line {current_line}"),
    }

    try:
        # Use binary mode to prevent universal newline translation interference
        with open(filepath, 'rb') as fb:
            # Wrap binary stream with a text reader using the specified encoding
            # errors='ignore' is pragmatic for old files, but 'strict' is safer
            with io.TextIOWrapper(fb, encoding=FILE_ENCODING, errors='replace', newline='') as f:
                while True:
                    # Find the start of the next section
                    pos = f.tell()
                    char = read_letter_sim(f)

                    if char is None: # Normal EOF
                        break
                    if char == '$': # End of Area list marker
                        print("Found '$' area list terminator.")
                        # Check if there's anything else unexpected
                        remaining_content = f.read()
                        if remaining_content and not remaining_content.isspace():
                             print(f"Warning: Content found after '$' terminator: {remaining_content[:50]}...")
                        break

                    if char != '#':
                        # Ignore content outside sections unless debugging
                        # print(f"Warning: Unexpected character '{char}' outside section near line {current_line}. Skipping to next '#'.")
                        read_to_eol_sim(f)
                        continue

                    # Read the section keyword
                    section_keyword = read_word_sim(f, "Section Keyword").upper()
                    if VERBOSE: print(f"Found section: #{section_keyword} at line {current_line}")

                    # Call the appropriate parser
                    if section_keyword in section_parsers:
                        section_parsers[section_keyword](f)
                    else:
                        print(f"Warning: Unrecognized section '#{section_keyword}' near line {current_line}. Attempting to skip.")
                        # Simple skip: Find the next '#' or '$' or EOF
                        while True:
                             s_pos = f.tell()
                             s_char = read_letter_sim(f)
                             if s_char is None or s_char in '#$':
                                  if s_char: f.seek(s_pos) # Put back marker
                                  break
                             read_to_eol_sim(f)


    except AreaParseException as e:
        print(e, file=sys.stderr)
        errors_found += 1
    except FileNotFoundError:
        print(f"Error: File not found: {filepath}", file=sys.stderr)
        errors_found += 1
    except Exception as e:
        print(f"Unexpected Python Error processing {filepath} near line {current_line}: {e.__class__.__name__}: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        errors_found += 1

    print(f"--- Finished {filepath} ---")
    print(f"Simulated Buffer Usage: {current_buffer_usage} / {MAX_STRING_BUFFER} bytes")
    if errors_found > 0:
        print(f"Result: {errors_found} CRITICAL ERROR(S) FOUND.")
    else:
        print("Result: Validation PASSED (based on implemented checks).")
    print("-" * 30)
    return errors_found == 0


# --- Main Execution ---
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Validate MUD .are files against C fread_string logic.")
    parser.add_argument("paths", nargs='+', help="List of .are files or directories containing them.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose parsing output.")
    parser.add_argument("-e", "--encoding", default=FILE_ENCODING, help=f"Specify file encoding (default: {FILE_ENCODING}).")

    args = parser.parse_args()

    VERBOSE = args.verbose
    FILE_ENCODING = args.encoding

    files_to_check = []
    for path in args.paths:
        if os.path.isfile(path) and path.lower().endswith(".are"):
            files_to_check.append(path)
        elif os.path.isdir(path):
            for filename in os.listdir(path):
                if filename.lower().endswith(".are"):
                    files_to_check.append(os.path.join(path, filename))

    if not files_to_check:
        print("No .are files found in the specified paths.")
        sys.exit(1)

    print(f"Starting validation for {len(files_to_check)} .are file(s)...")
    print(f"Using Encoding: {FILE_ENCODING}")
    print(f"Simulated MAX_STRING: {MAX_STRING_BUFFER}")
    print("-" * 30)

    total_passed = 0
    total_failed = 0

    for filepath in files_to_check:
        if validate_area_file(filepath):
            total_passed += 1
        else:
            total_failed += 1

    print("=" * 30)
    print("Validation Summary:")
    print(f" Files Checked: {len(files_to_check)}")
    print(f" Files Passed:  {total_passed}")
    print(f" Files Failed:  {total_failed}")
    print("=" * 30)

    if total_failed > 0:
        print("\nReview the CRITICAL ERRORS reported above.")
        sys.exit(1)
    else:
        print("\nAll checked files passed validation according to the script's logic.")
        sys.exit(0)