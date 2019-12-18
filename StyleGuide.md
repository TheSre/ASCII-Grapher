# Style Guide
* line length: no more than 80 characters
    * when continuing a line, double indent the new lines
* functions: 
    * aim for 15 lines or shorter, with a hard cap at 30 lines
    * should do one thing
    * try to minimize the number of function parameters
    * if we really wanna be next-level, we could try to avoid functions having
      side effects, but I don't think that needs to be a hard & fast rule
* identifiers: 
    * use names that clearly describe the purpose of the thing being identified
        * don't use single letter names except for common idioms (like `i`, `j`,
          `k` for loop counters)
        * don't differentiate variables with arbitrary or meaningless
          differences in their identifiers (e.g. `intA`, `intB`)
    * generally, it's probably unnecessary to use the variable's type in its
      name (e.g. `inputString` for a string read in from the user)
    * use pronounceable names (goal is for the code to read like prose)
    * "normal" variables and function names should be in `camelCase`
    * constant variables should be in all caps with underscores separating words
      (e.g. `A_CONSTANT_VARIABLE`)
    * class names (not relevant for C) should be in `PascalCase`
    * generally, replace literal values with constants (more searchable &
      reveals meaning--e.g. `MAX_INPUT_LENGTH` instead of the value 100)
* comments:
    * it's better to have good identifier names than to have to give a comment
    * for nontrivial or unintuitive functions however, it's probably good to
      include a brief comment describing what it does
    * use brief comments to explain unintuitive or surprising lines
    * if you want to indicate that something still needs doin', leave a `//
      TODO:` comment (for fixin', `// FIXME:`)
        * these are nicely highlighted by most editors
* formatting:
    * horizontal spacing: 
        * use spaces to make code more readable (e.g. `i += 5` instead of `i+=5`)
        * use indentation to indicate levels of nesting
    * vertical spacing can be used to separate sections that are conceptually /
      functionally different
    * use [K&R style](https://en.wikipedia.org/wiki/Indentation_style#K&R_style) for braces
        * makes it easier to see where scopes line up
    * when line containing function declaration is too long, double indent all
      parameters on separate lines, e.g.:
      ```
      int longFunctionNameGoesRightHereInThisGeneralVicinity(
                 char *accompaniedByALongFirstParameterName,
                 int alongWithALongSecondParameterName)
      ```
      Instead of:
      ```
      int longFunctionNameGoesRightHereInThisGeneralVicinity(char *accompaniedByALongFirstParameterName,
                 int alongWithALongSecondParameterName)

      ```

* use vim or emacs to write your code (don't @ me)
