#ifndef KEYS_H
#define KEYS_H

/* Key macros */
/* These macros are not guaranteed to be defined,
 *  however we wish to test for these anyways while
 *  interpretting user commands.
 * Input values are guaranteed to be postive,
 *  so setting them to -1 means the test always just silently fail,
 *  but compile when the they are not supported means of input.
 */
#define KEY_UNDEF_BASE 0

#ifndef KEY_DOWN
# define KEY_DOWN KEY_UNDEF_BASE-1
#endif
#ifndef KEY_UP
# define KEY_UP KEY_UNDEF_BASE-2
#endif
#ifndef KEY_LEFT
# define KEY_LEFT KEY_UNDEF_BASE-3
#endif
#ifndef KEY_RIGHT
# define KEY_RIGHT KEY_UNDEF_BASE-4
#endif
#ifndef KEY_HOME
# define KEY_HOME _KEY_UNDEF_BASE-5
#endif
#ifndef KEY_LL
# define KEY_LL    KEY_UNDEF_BASE-6
#endif
#ifndef KEY_PPAGE
# define KEY_PPAGE KEY_UNDEF_BASE-7
#endif
#ifndef KEY_NPAGE
# define KEY_NPAGE KEY_UNDEF_BASE-8
#endif
#ifndef KEY_ENTER
# define KEY_ENTER KEY_UNDEF_BASE-9
#endif
#ifndef KEY_CLEAR
# define KEY_CLEAR KEY_UNDEF_BASE-10
#endif
#ifndef KEY_RESIZE
# define KEY_RESIZE KEY_UNDEF_BASE-11
#endif
#ifndef KEY_END
# define KEY_END KEY_UNDEF_BASE-12
#endif

/* Always define these keys */
#ifndef ESC
# define    ESC    '\033'		/* escape character */
#endif
#ifndef DEL
# define    DEL    '\177'		/* delete character */
#endif


#endif /* KEYS_H*/
