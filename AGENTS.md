# Style guide

Functions:

- The first function in the file should be main, or the primary overview function
- Keep main or the primary overview function minimal, have it just call functions mostly
- Functions go in the file in the order they are called, so reads like a book, with utils at end

Comments:

- Every block of code needs a comment above it, with a blank line above the comment
- No line of code should exist just by itself, just put a comment above it or group it
- Always insert a blank line immediately before every comment block, except for at top of functions
- No need for blank lines at the start of functions above the first comment
- Comments are short overviews of what the block does, like "// Create thing", not passive "// Thing"
- No doc strings, just put a single line comment above each function
- No parens in comments, use commas instead
- No comments at end of lines

Code:

- Allow running the program with no args to do something useful, so have defaults
- Fix or supress any warnings that occur in output, working run should be totall clean
- Try to not have value defaults in function sigs, put them in consts at top
- Function calls should be on one line, not broken over many lines
- Remove any imports or includes that are not used

Naming:

- No variable or function name short abbreviations, e.g. ok to use config, but not cfg
- Keep code simple and minimal number of lines
- No underscores in front of functions
- Use full variable and function names
- Prefer argument names load and save



# Project 

- Read the file before editing, the user often tweaks comments and structure between turns
- Only change what was asked for, preserve the user's wording and manual edits
- Arduino sketches live in one folder per sketch, folder name matches the .ino file name
- Close Serial Monitor before upload



