#pragma once

#define DEBUG_RESET_COLOURS "\x1b[0m"
#define DEBUG_BLACK_PEN "\x1b[30m"
#define DEBUG_RED_PEN "\x1b[31m"
#define DEBUG_GREEN_PEN "\x1b[32m"
#define DEBUG_YELLOW_PEN "\x1b[33m"
#define DEBUG_BLUE_PEN "\x1b[34m"
#define DEBUG_MAGENTA_PEN "\x1b[35m"
#define DEBUG_CYAN_PEN "\x1b[36m"
#define DEBUG_WHITE_PEN "\x1b[37m"

#define DEBUG_BLACK_PAPER "\x1b[40m"
#define DEBUG_RED_PAPER "\x1b[41m"
#define DEBUG_GREEN_PAPER "\x1b[42m"
#define DEBUG_YELLOW_PAPER "\x1b[43m"
#define DEBUG_BLUE_PAPER "\x1b[44m"
#define DEBUG_MAGENTA_PAPER "\x1b[45m"
#define DEBUG_CYAN_PAPER "\x1b[46m"
#define DEBUG_WHITE_PAPER "\x1b[47m"

#define DEBUG_CLR_SCREEN "\x1b[2J"
#define DEBUG_CLR_SCREEN_CURSOR_TO_BEGIN "\x1b[1J"
#define DEBUG_CLR_SCREEN_CURSOR_TO_END "\x1b[0J"

#define DEBUG_CLR_LINE "\x1b[2dK"
#define DEBUG_CLR_LINE_CURSOR_TO_BEGIN "\x1b[1dK"
#define DEBUG_CLR_LINE_CURSOR_TO_END "\x1b[0dK"

#define DEBUG_CURSOR_UP "\x1b[1A"
#define DEBUG_CURSOR_DOWN "\x1b[1B"
#define DEBUG_CURSOR_LEFT "\x1b[1C"
#define DEBUG_CURSOR_RIGHT "\x1b[1D"