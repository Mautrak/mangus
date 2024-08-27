# Uzak Diyarlar MUD - Coding Guidelines and Conventions

This document outlines the coding guidelines and conventions used in the Uzak Diyarlar MUD project. Adhering to these guidelines will help maintain consistency across the codebase and make it easier for developers to understand and modify the code.

## General Principles

1. Clarity: Write code that is easy to read and understand.
2. Consistency: Follow established patterns and conventions within the project.
3. Modularity: Organize code into logical, reusable components.
4. Documentation: Provide clear comments and documentation for complex logic.

## Coding Style

### Indentation and Formatting

- Use 4 spaces for indentation (no tabs).
- Place opening braces on the same line as the control statement.
- Use spaces around operators and after commas.

Example:
```c
if (condition) {
    // Code here
} else {
    // More code
}
```

### Naming Conventions

- Use snake_case for function and variable names.
- Use UPPER_CASE for constants and macros.
- Prefix global variables with `g_`.
- Use descriptive names that clearly indicate the purpose of the variable or function.

Example:
```c
void update_player_status(CHAR_DATA *ch);
#define MAX_LEVEL 100
extern int g_server_port;
```

### Function Design

- Keep functions focused on a single task.
- Aim for functions to be no longer than 50 lines. If a function grows too large, consider breaking it into smaller, more manageable pieces.
- Use meaningful parameter names.

Example:
```c
bool apply_spell_effect(CHAR_DATA *caster, CHAR_DATA *target, int spell_id);
```

## Documentation

### File Headers

Each source file should begin with a header comment that includes:
- Brief description of the file's contents
- Author information
- Last modification date
- Any copyright or license information

Example:
```c
/*
 * File: combat.c
 * Description: Main combat routines for Uzak Diyarlar MUD
 * Author: [Your Name]
 * Last Modified: [Date]
 * 
 * Copyright (c) [Year], Uzak Diyarlar MUD. All rights reserved.
 */
```

### Function Comments

- Provide a brief description of what the function does.
- Document parameters and return values.
- Note any important side effects or exceptions.

Example:
```c
/*
 * Apply damage to a character and check for death.
 * 
 * Parameters:
 *   ch - The character taking damage
 *   damage - Amount of damage to apply
 * 
 * Returns:
 *   true if the character died, false otherwise
 */
bool apply_damage(CHAR_DATA *ch, int damage);
```

### Inline Comments

- Use inline comments to explain complex logic or non-obvious code.
- Keep inline comments brief and to the point.

## Error Handling

- Use consistent error handling mechanisms throughout the codebase.
- Check return values from functions that can fail.
- Use meaningful error messages that help in debugging.

Example:
```c
FILE *file = fopen(filename, "r");
if (file == NULL) {
    log_error("Failed to open file: %s", filename);
    return ERROR_FILE_NOT_FOUND;
}
```

## Memory Management

- Always free allocated memory when it's no longer needed.
- Use the project's custom memory management functions consistently.
- Be mindful of potential memory leaks, especially in long-running loops or frequently called functions.

## Portability

- Avoid platform-specific code unless absolutely necessary.
- If platform-specific code is required, use conditional compilation and clearly document the affected platforms.

Example:
```c
#ifdef _WIN32
    // Windows-specific code
#else
    // Unix-like systems code
#endif
```

## Version Control Practices

- Write clear, concise commit messages that explain the purpose of the change.
- Keep commits focused on a single logical change.
- Use feature branches for developing new features or major changes.

## Testing

- Write unit tests for new functionality where possible.
- Manually test changes thoroughly before committing.
- Consider adding integration tests for complex interactions between systems.

## Modernization Notes

As the project aims to modernize, consider the following:

1. Gradual migration to more modern C standards (e.g., C99 or C11) where beneficial.
2. Evaluation of third-party libraries for complex tasks (e.g., better string handling, more efficient data structures).
3. Improving type safety by using more specific types and avoiding generic pointers where possible.
4. Implementing better error handling mechanisms, possibly using a more structured approach.

By following these guidelines, we can maintain a consistent, readable, and maintainable codebase for Uzak Diyarlar MUD. Remember that these guidelines may evolve over time, and constructive discussions about improving our coding practices are always welcome.