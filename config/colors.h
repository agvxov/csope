#ifndef CONFIG_H
#define CONFIG_H

/* List of color options:
			COLOR_BLACK
			COLOR_RED
			COLOR_GREEN
			COLOR_YELLOW
			COLOR_BLUE
			COLOR_MAGENTA
			COLOR_CYAN
			COLOR_WHITE
			-1				// for transparent (only works if that is your default terminal background)
*/

/* --- Valid presets --- */
#define MONOCHROME_PRESET		1
#define COLORFUL_PRESET			2
// Your own preset.
//  NOTE: you have to assign a color
//         to each end every macro
#define COLORFUL_CUSTOM			3

/* --- Preset selection --- */
// Set to one of the numerical values above
#define COLOR_PRESET	2

#if COLOR_PRESET == 1
#	define COLOR_STD_FG							COLOR_WHITE
#	define COLOR_STD_BG							COLOR_BLACK
#	define COLOR_FRAME_FG                       COLOR_WHITE
#	define COLOR_FRAME_BG                       COLOR_BLACK
#	define COLOR_PROMPT_FG                      COLOR_WHITE
#	define COLOR_PROMPT_BG                      COLOR_BLACK
#	define COLOR_CURSOR                         COLOR_WHITE
#	define COLOR_FIELD_FG                       COLOR_WHITE
#	define COLOR_FIELD_BG                       COLOR_BLACK
#	define COLOR_FIELD_SELECTED_FG              COLOR_WHITE
#	define COLOR_FIELD_SELECTED_BG              COLOR_BLACK
#	define COLOR_HELP_FG                        COLOR_WHITE
#	define COLOR_HELP_BG                        COLOR_BLACK
#	define COLOR_TOOLTIP_FG                     COLOR_WHITE
#	define COLOR_TOOLTIP_BG                     COLOR_BLACK
#	define COLOR_CASE_FG                        COLOR_WHITE
#	define COLOR_CASE_BG                        COLOR_BLACK
#	define COLOR_MESSAGE_FG                     COLOR_WHITE
#	define COLOR_MESSAGE_BG                     COLOR_BLACK
#	define COLOR_PATTERN_FG                     COLOR_WHITE
#	define COLOR_PATTERN_BG                     COLOR_BLACK
#	define COLOR_TABLE_HEADER_FG                COLOR_WHITE
#	define COLOR_TABLE_HEADER_BG                COLOR_BLACK
#	define COLOR_TABLE_ID_FG                    COLOR_WHITE
#	define COLOR_TABLE_ID_BG                    COLOR_BLACK
#	define COLOR_TABLE_MARK_FG                  COLOR_WHITE
#	define COLOR_TABLE_MARK_BG                  COLOR_BLACK
#	define COLOR_TABLE_COL_FILE_FG              COLOR_WHITE
#	define COLOR_TABLE_COL_FILE_BG              COLOR_BLACK
#	define COLOR_TABLE_COL_FUNCTION_FG          COLOR_WHITE
#	define COLOR_TABLE_COL_FUNCTION_BG          COLOR_BLACK
#	define COLOR_TABLE_COL_LINE_FG              COLOR_WHITE
#	define COLOR_TABLE_COL_LINE_BG              COLOR_BLACK
#	define COLOR_TABLE_COL_TEXT_FG              COLOR_WHITE
#	define COLOR_TABLE_COL_TEXT_BG              COLOR_BLACK
#	define COLOR_TABLE_SELECTED_ID_FG           COLOR_WHITE
#	define COLOR_TABLE_SELECTED_ID_BG           COLOR_BLACK
#	define COLOR_TABLE_SELECTED_MARK_FG         COLOR_WHITE
#	define COLOR_TABLE_SELECTED_MARK_BG         COLOR_BLACK
#	define COLOR_TABLE_COL_SELECTED_FILE_FG     COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_FILE_BG     COLOR_BLACK
#	define COLOR_TABLE_COL_SELECTED_FUNCTION_FG COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_FUNCTION_BG COLOR_BLACK
#	define COLOR_TABLE_COL_SELECTED_LINE_FG     COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_LINE_BG     COLOR_BLACK
#	define COLOR_TABLE_COL_SELECTED_TEXT_FG     COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_TEXT_BG     COLOR_BLACK
#	define COLOR_PAGER_MSG_FG                   COLOR_WHITE
#	define COLOR_PAGER_MSG_BG                   COLOR_BLACK
#elif COLOR_PRESET == 2
#	define COLOR_STD_FG							COLOR_WHITE
#	define COLOR_STD_BG							-1
#	define COLOR_FRAME_FG						COLOR_GREEN
#	define COLOR_FRAME_BG						-1
#	define COLOR_PROMPT_FG						COLOR_BLUE
#	define COLOR_PROMPT_BG						-1
#	define COLOR_CURSOR							COLOR_WHITE
#	define COLOR_FIELD_FG						COLOR_WHITE
#	define COLOR_FIELD_BG						-1
#	define COLOR_FIELD_SELECTED_FG				COLOR_BLACK
#	define COLOR_FIELD_SELECTED_BG				COLOR_WHITE
#	define COLOR_HELP_FG						COLOR_YELLOW
#	define COLOR_HELP_BG						-1
#	define COLOR_TOOLTIP_FG						COLOR_WHITE
#	define COLOR_TOOLTIP_BG						COLOR_GREEN
#	define COLOR_CASE_FG						COLOR_GREEN
#	define COLOR_CASE_BG						-1
#	define COLOR_MESSAGE_FG						COLOR_WHITE
#	define COLOR_MESSAGE_BG						COLOR_BLACK
#	define COLOR_PATTERN_FG						COLOR_WHITE
#	define COLOR_PATTERN_BG						-1
#	define COLOR_TABLE_HEADER_FG				COLOR_YELLOW
#	define COLOR_TABLE_HEADER_BG				-1
#	define COLOR_TABLE_ID_FG					COLOR_CYAN
#	define COLOR_TABLE_ID_BG					-1
#	define COLOR_TABLE_MARK_FG					COLOR_WHITE
#	define COLOR_TABLE_MARK_BG					-1
#	define COLOR_TABLE_COL_FILE_FG				COLOR_MAGENTA
#	define COLOR_TABLE_COL_FILE_BG				-1
#	define COLOR_TABLE_COL_FUNCTION_FG			COLOR_RED
#	define COLOR_TABLE_COL_FUNCTION_BG			-1
#	define COLOR_TABLE_COL_LINE_FG				COLOR_CYAN
#	define COLOR_TABLE_COL_LINE_BG				-1
#	define COLOR_TABLE_COL_TEXT_FG				COLOR_GREEN
#	define COLOR_TABLE_COL_TEXT_BG				-1
#	define COLOR_TABLE_SELECTED_ID_FG			COLOR_CYAN
#	define COLOR_TABLE_SELECTED_ID_BG			COLOR_WHITE
#	define COLOR_TABLE_SELECTED_MARK_FG			COLOR_BLACK
#	define COLOR_TABLE_SELECTED_MARK_BG			COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_FILE_FG		COLOR_MAGENTA
#	define COLOR_TABLE_COL_SELECTED_FILE_BG		COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_FUNCTION_FG	COLOR_RED
#	define COLOR_TABLE_COL_SELECTED_FUNCTION_BG	COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_LINE_FG		COLOR_CYAN
#	define COLOR_TABLE_COL_SELECTED_LINE_BG		COLOR_WHITE
#	define COLOR_TABLE_COL_SELECTED_TEXT_FG		COLOR_GREEN
#	define COLOR_TABLE_COL_SELECTED_TEXT_BG		COLOR_WHITE
#	define COLOR_PAGER_MSG_FG					COLOR_YELLOW
#	define COLOR_PAGER_MSG_BG					COLOR_WHITE
#elif COLOR_PRESET == 3
#	define COLOR_STD_FG							
#	define COLOR_STD_BG							
#	define COLOR_FRAME_FG						
#	define COLOR_FRAME_BG						
#	define COLOR_PROMPT_FG						
#	define COLOR_PROMPT_BG						
#	define COLOR_CURSOR							
#	define COLOR_FIELD_FG						
#	define COLOR_FIELD_BG						
#	define COLOR_FIELD_SELECTED_FG				
#	define COLOR_FIELD_SELECTED_BG				
#	define COLOR_HELP_FG						
#	define COLOR_HELP_BG						
#	define COLOR_TOOLTIP_FG						
#	define COLOR_TOOLTIP_BG						
#	define COLOR_CASE_FG						
#	define COLOR_CASE_BG						
#	define COLOR_MESSAGE_FG						
#	define COLOR_MESSAGE_BG						
#	define COLOR_PATTERN_FG						
#	define COLOR_PATTERN_BG						
#	define COLOR_TABLE_HEADER_FG				
#	define COLOR_TABLE_HEADER_BG				
#	define COLOR_TABLE_ID_FG					
#	define COLOR_TABLE_ID_BG					
#	define COLOR_TABLE_MARK_FG					
#	define COLOR_TABLE_MARK_BG					
#	define COLOR_TABLE_COL_FILE_FG				
#	define COLOR_TABLE_COL_FILE_BG				
#	define COLOR_TABLE_COL_FUNCTION_FG			
#	define COLOR_TABLE_COL_FUNCTION_BG			
#	define COLOR_TABLE_COL_LINE_FG				
#	define COLOR_TABLE_COL_LINE_BG				
#	define COLOR_TABLE_COL_TEXT_FG				
#	define COLOR_TABLE_COL_TEXT_BG				
#	define COLOR_TABLE_SELECTED_ID_FG			
#	define COLOR_TABLE_SELECTED_ID_BG			
#	define COLOR_TABLE_SELECTED_MARK_FG			
#	define COLOR_TABLE_SELECTED_MARK_BG			
#	define COLOR_TABLE_COL_SELECTED_FILE_FG		
#	define COLOR_TABLE_COL_SELECTED_FILE_BG		
#	define COLOR_TABLE_COL_SELECTED_FUNCTION_FG	
#	define COLOR_TABLE_COL_SELECTED_FUNCTION_BG	
#	define COLOR_TABLE_COL_SELECTED_LINE_FG		
#	define COLOR_TABLE_COL_SELECTED_LINE_BG		
#	define COLOR_TABLE_COL_SELECTED_TEXT_FG		
#	define COLOR_TABLE_COL_SELECTED_TEXT_BG		
#	define COLOR_PAGER_MSG_FG					
#	define COLOR_PAGER_MSG_BG					
#else
#	error "Color profile not valid"
#endif



/* ############################################### */
/* ### Don't touch if you are just configuring ### */
enum color_pairs{
	COLOR_PAIR_STD = 1,
	COLOR_PAIR_FRAME,
	COLOR_PAIR_PROMPT,
	COLOR_PAIR_FIELD,
	COLOR_PAIR_FIELD_SELECTED,
	COLOR_PAIR_HELP,
	COLOR_PAIR_TOOLTIP,
	COLOR_PAIR_CASE,
	COLOR_PAIR_PATTERN,
	COLOR_PAIR_MESSAGE,
	COLOR_PAIR_TABLE_HEADER,
	COLOR_PAIR_TABLE_ID,
	COLOR_PAIR_TABLE_MARK,
	COLOR_PAIR_TABLE_COL_FILE,
	COLOR_PAIR_TABLE_COL_FUNCTION,
	COLOR_PAIR_TABLE_COL_LINE,
	COLOR_PAIR_TABLE_COL_TEXT,
	COLOR_PAIR_TABLE_SELECTED_ID,
	COLOR_PAIR_TABLE_SELECTED_MARK,
	COLOR_PAIR_TABLE_COL_SELECTED_FILE,
	COLOR_PAIR_TABLE_COL_SELECTED_FUNCTION,
	COLOR_PAIR_TABLE_COL_SELECTED_LINE,
	COLOR_PAIR_TABLE_COL_SELECTED_TEXT,
	COLOR_PAIR_PAGER_MSG
};

#define easy_init_pair(x) init_pair(COLOR_PAIR_ ## x, COLOR_ ## x ## _FG, COLOR_ ## x ## _BG)

/* Other options:
			A_NORMAL			: Normal display (no highlight)
			A_UNDERLINE			: Underlining
			A_REVERSE			: Reverse video
			A_BLINK				: Blinking
			A_BOLD				: Extra bright or bold
			A_STANDOUT			: Best highlighting mode of the terminal.
	NOTE: you can specify more than one by separating the options by a '|' sign.
		{ A_BLINK | A_BOLD }
*/
#define ATTRIBUTE_FIELD_SELECTED		A_BOLD
#define ATTRIBUTE_RESULT_SELECTED		A_BOLD

#endif
