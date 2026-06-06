+ what the hell is the deal with `source/gscope.c`?
+ sort out the global hell
+ lineflagafterfile is stupid; a format string is infinitly better
+ INCDIR is dump, we should read the include paths normally
+ scrollbar() uses magic int literals?
+ was there really ever a scrollbar?
+ a search struct could be great for caching and could ease the global situation
+ merge support for ctags
+ add project summary page(?)
+ add basic search globbing
+ add search filters
+ line mode should be like a shell; that would allow us to remove the caseless cli flag
+ -X seems mostly useless; if you want csope to not shitup your $CWD,
   then that should be accomplished with a designated (but optional) dump dir (similar to vim undodir)
+ listfile should accept the same syntax git does, not because i want to such up to git, but because its extensively sane
+ path.c should be replaced with a static lib

### From the original
+ Same capabilities as interactive in non interactive (one shot) mode
+ Provide some how-do-I-use-this-thing doc.
