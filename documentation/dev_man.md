# Csope developer manual

## Control flow
...

## Project structure
| Component | Purpose |
| :-------: | :-----: |
| main.c    | generic init functions, main() and primary event loops (and junk) |
| display.c | all functions directly dealing with NCurses |
| input.c   | top layer of functions dealing with user input; migth dispatch to readline |
| opt.c     | reads initialization options from cli/env |
| find.c    | searching functions |
| readline.c | all functions directly dealing with GNU Readline; responsible for line editing in *input mode* |
| help.c    | all functions dealing with help messages |
| auto\_vararg.h | vararg argument counter macro hack |
| vpath.c   | Make like VPATH support |
| refsfound | XXX: ducktaping to page the cscope database |
| build.c   | XXX: functions required for building the cscope database |
| globals.h | XXX: an inherited curse; global var/prototype hell |
| command.c | XXX: clusterfuck |
| mouse.c   | XXX: legacy, non-functional (curses) mouse support

## global.h
`global.h` was the largest dragdown of the original code base.
It had well over a hundred global symbols being exposed to the entire codebase.
The problem being that each was used (and sometimes defined) indiscriminately.
Every subsystem would call into every other
and manipulating globals was more common than passing arguments.

Therefor, a K&R header structure is being slowly adopted. WIP.

However, `global.h` is not to be replaced completely.
It rightfully holds macro utilities and macro symbol overrides.
For example, `fopen()` of the standard library is silently replaced with `_myfopen()`,
so that its impossible to forget about close-on-exec.

## Key Symbols
| Global | Role |
| :----: | :--: |
| int input_mode | Responsible of keeping track how current input should be handled. Not only does  the readline handler depend on it, its also used to determine what types of inputs all legal (e.g. swapping to another window). Takes up on of the values of the INPUT_\* macros.
| int window_change | Bit mask type of the CH_\* macros. Keeps track of the windows to be refresed on the next run of display(). Could be better utalized.

## Notes

### OGS
CScope used to support something
refered to as "OGS book and subsystem names".
I have literally no clue what OGS is in reference to,
it may or may not be the Quake II engine.
Based on the code,
the idea is that folders ending `.ss/` are "subsystems",
and folders below subsystems are "books".
CScope would display the subsystem and book names
in its table view as any other field,
given that the `ogs` global was set.
So how do you set the ogs global?
You don't,
it was never documented,
and there was never a way to set it at run time.
I assume one project wanted it
and sneaked in the patch.

It has been removed.

### Reference files
The `cscope.out` files which contain symbol database are called cross-reference files.
The code refers to these as ref files.
The concrete format of a wild ref file could be a number of things:
* each cscope has a corresponding file format version, which may or may not be the same as the previous
* ref generation accepted a number of options
    + compression (space efficiency)
    + inversion (time efficiency)
    + truncate symbol names (space efficiency with the danger of false results, unless of course you are a `lthxr`)

Cscope 15.9 has some compatibility code for all ref versions,
but I doubt that anything below 8 would actually work.
Now, this compatibility layer must have been nice
when building cross references would take considerable amounts of time
and cscope was under rapid development,
however both times are gone now.

The way ref file options are implemented is a huge mess.
It would be possible to clean it,
however I do not believe anyone has used actually used them in the past 10 years.

How the situation has progress so far:
* the Truncation option has been removed
* fileversion conditional IO has been removed, the remnants are only there to signal an error

### List files
Csope can accept a list file which specifies what the source files are of a project.
Cscope had `cscope.files`; a file that the user was expected to write by hand.
It would have the following syntax:
```
listfile := line+
line := option* filename
option := -I | -c | -k | -p | -T
filename := "string" | string
```

The fun thing about the options being that they modify global state,
so it might as well could have been a config file,
but it wasn't.

Modifying state such a way was deemed undesirable.
Therefor only -I is allowed until the subsystem is replaced.

### Edit all
\<Ctrl\>+e opens all results for editing in a loop.
Between edits there was a prompt to interrupt the iteration.
There was an `-e` option which to suppress this behaviour (also making you to break).
Both models proved to be annoying, therefor now we check for the editor returning non-zero.
