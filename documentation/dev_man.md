# Control flow
...

# Project structure	/*probably move to documentation*/
| Component | Purpose |
| :-------: | :-----: |
| main.c | generic init functions, main() and primary event loops (and junk) |
| display.c | all functions directly dealing with NCurses |
| input.c | top layer of functions dealing with user input; migth dispatch to readline |
| find.c | searching functions |
| globals.h | an inherited curse; global var/prototype hell |
| readline.c | all functions directly dealing with GNU Readline; responsible for line editing in *input mode* |
| help.c | all functions dealing with help messages |

# Key Symbols
| Global | Role |
| :----: | :--: |
| int input_mode | Responsible of keeping track how current input should be handled. Not only does  the readline handler depend on it, its also used to determine what types of inputs all legal (e.g. swapping to another window). Takes up on of the values of the INPUT_\* macros.
| int window_change | Bit mask type of the CH_\* macros. Keeps track of the windows to be refresed on the next run of display(). Could be better utalized.

# Notes
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
