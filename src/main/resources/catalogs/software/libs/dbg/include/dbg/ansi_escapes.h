#pragma once

// SGR (Select Graphic Rendition) commands
#define ANSI_RESET_ATTRIBUTES "\x1b[0m"
#define ANSI_BRIGHT_ON "\x1b[1m"
#define ANSI_BRIGHT_OFF "\x1b[22m"
#define ANSI_BLINK_ON "\x1b[5m"
#define ANSI_BLINK_OFF "\x1b[25m"

#define ANSI_BLACK_PEN "\x1b[30m"
#define ANSI_RED_PEN "\x1b[31m"
#define ANSI_GREEN_PEN "\x1b[32m"
#define ANSI_YELLOW_PEN "\x1b[33m"
#define ANSI_BLUE_PEN "\x1b[34m"
#define ANSI_MAGENTA_PEN "\x1b[35m"
#define ANSI_CYAN_PEN "\x1b[36m"
#define ANSI_WHITE_PEN "\x1b[37m"

#define ANSI_BLACK_PAPER "\x1b[40m"
#define ANSI_RED_PAPER "\x1b[41m"
#define ANSI_GREEN_PAPER "\x1b[42m"
#define ANSI_YELLOW_PAPER "\x1b[43m"
#define ANSI_BLUE_PAPER "\x1b[44m"
#define ANSI_MAGENTA_PAPER "\x1b[45m"
#define ANSI_CYAN_PAPER "\x1b[46m"
#define ANSI_WHITE_PAPER "\x1b[47m"


// Erase in Display Commands
#define ANSI_CLR_SCREEN "\x1b[2J"
#define ANSI_CLR_SCREEN_CURSOR_TO_BEGIN "\x1b[1J"
#define ANSI_CLR_SCREEN_CURSOR_TO_END "\x1b[0J"

// Erase in Line Commands
#define ANSI_CLR_LINE "\x1b[2dK"
#define ANSI_CLR_LINE_CURSOR_TO_BEGIN "\x1b[1dK"
#define ANSI_CLR_LINE_CURSOR_TO_END "\x1b[0dK"

// Cursor commands
#define ANSI_CURSOR_UP "\x1b[1A"
#define ANSI_CURSOR_DOWN "\x1b[1B"
#define ANSI_CURSOR_LEFT "\x1b[1C"
#define ANSI_CURSOR_RIGHT "\x1b[1D"

// Cursor Position commands (AKA Home)
#define ANSI_CURSOR_HOME "\x1b[;H"
#define ANSI_CURSOR_POSITION_STRINGIZER(s) #s
#define ANSI_CURSOR_POSITION(col, row) "\x1b[" ANSI_CURSOR_POSITION_STRINGIZER(row) ";" ANSI_CURSOR_POSITION_STRINGIZER(col) "H"

