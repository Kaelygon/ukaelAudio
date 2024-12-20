//key escape sequences
#ifndef KAELKEYCONST_H
#define KAELKEYCONST_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>


#define KEY_NULL						((uint8_t[]){0,0})
#define KEY_CTRL_A						((uint8_t[]){1,0})
#define KEY_CTRL_B						((uint8_t[]){2,0})
#define KEY_CTRL_C						((uint8_t[]){3,0})
#define KEY_CTRL_D						((uint8_t[]){4,0})
#define KEY_CTRL_E						((uint8_t[]){5,0})
#define KEY_CTRL_F						((uint8_t[]){6,0})
#define KEY_CTRL_G						((uint8_t[]){7,0})
#define KEY_DELETE_WORD					((uint8_t[]){8,0})
#define KEY_CTRL_H						((uint8_t[]){8,0})
#define KEY_TAB							((uint8_t[]){9,0})
#define KEY_CTRL_I						((uint8_t[]){9,0})
#define KEY_CTRL_J						((uint8_t[]){10,0})
#define KEY_CTRL_K						((uint8_t[]){11,0})
#define KEY_CTRL_L						((uint8_t[]){12,0})
#define KEY_CTRL_M						((uint8_t[]){13,0})
#define KEY_CTRL_N						((uint8_t[]){14,0})
#define KEY_CTRL_O						((uint8_t[]){15,0})
#define KEY_CTRL_P						((uint8_t[]){16,0})
#define KEY_CTRL_Q						((uint8_t[]){17,0})
#define KEY_CTRL_R						((uint8_t[]){18,0})
#define KEY_CTRL_S						((uint8_t[]){19,0})
#define KEY_CTRL_T						((uint8_t[]){20,0})
#define KEY_CTRL_U						((uint8_t[]){21,0})
#define KEY_CTRL_V						((uint8_t[]){22,0})
#define KEY_CTRL_W						((uint8_t[]){23,0})
#define KEY_CTRL_X						((uint8_t[]){24,0})
#define KEY_CTRL_Y						((uint8_t[]){25,0})
#define KEY_CTRL_Z						((uint8_t[]){26,0})
#define KEY_ENTER						((uint8_t[]){10,0})
#define KEY_ESCAPE						((uint8_t[]){27,0})
#define KEY_ALT_A						((uint8_t[]){27,97,0})
#define KEY_ALT_B						((uint8_t[]){27,98,0})
#define KEY_ALT_C						((uint8_t[]){27,99,0})
#define KEY_ALT_D						((uint8_t[]){27,100,0})
#define KEY_ALT_E						((uint8_t[]){27,101,0})
#define KEY_ALT_F						((uint8_t[]){27,102,0})
#define KEY_ALT_G						((uint8_t[]){27,103,0})
#define KEY_ALT_H						((uint8_t[]){27,104,0})
#define KEY_ALT_I						((uint8_t[]){27,105,0})
#define KEY_ALT_J						((uint8_t[]){27,106,0})
#define KEY_ALT_K						((uint8_t[]){27,107,0})
#define KEY_ALT_L						((uint8_t[]){27,108,0})
#define KEY_ALT_M						((uint8_t[]){27,109,0})
#define KEY_ALT_N						((uint8_t[]){27,110,0})
#define KEY_ALT_O						((uint8_t[]){27,111,0})
#define KEY_ALT_P						((uint8_t[]){27,112,0})
#define KEY_ALT_Q						((uint8_t[]){27,113,0})
#define KEY_ALT_R						((uint8_t[]){27,114,0})
#define KEY_ALT_S						((uint8_t[]){27,115,0})
#define KEY_ALT_T						((uint8_t[]){27,116,0})
#define KEY_ALT_U						((uint8_t[]){27,117,0})
#define KEY_ALT_V						((uint8_t[]){27,118,0})
#define KEY_ALT_W						((uint8_t[]){27,119,0})
#define KEY_ALT_X						((uint8_t[]){27,120,0})
#define KEY_ALT_Y						((uint8_t[]){27,121,0})
#define KEY_ALT_Z						((uint8_t[]){27,122,0})
#define KEY_F1							((uint8_t[]){27,79,80,0})
#define KEY_F2							((uint8_t[]){27,79,81,0})
#define KEY_F3							((uint8_t[]){27,79,82,0})
#define KEY_F4							((uint8_t[]){27,79,83,0})
#define KEY_F5							((uint8_t[]){27,91,49,53,126,0})
#define KEY_F6							((uint8_t[]){27,91,49,55,126,0})
#define KEY_F7							((uint8_t[]){27,91,49,56,126,0})
#define KEY_F8							((uint8_t[]){27,91,49,57,126,0})
#define KEY_INSERT						((uint8_t[]){27,91,50,126,0})
#define KEY_F9							((uint8_t[]){27,91,50,48,126,0})
#define KEY_F10							((uint8_t[]){27,91,50,49,126,0})
#define KEY_F11							((uint8_t[]){27,91,50,51,126,0})
#define KEY_F12							((uint8_t[]){27,91,50,52,126,0})
#define KEY_DELETE						((uint8_t[]){27,91,51,126,0})
#define KEY_PAGE_UP						((uint8_t[]){27,91,53,126,0})
#define KEY_PAGE_DOWN					((uint8_t[]){27,91,54,126,0})
#define KEY_UP_ARROW					((uint8_t[]){27,91,65,0})
#define KEY_UP_ARROW_NUM				((uint8_t[]){27,91,65,0})
#define KEY_DOWN_ARROW					((uint8_t[]){27,91,66,0})
#define KEY_DOWN_ARROW_NUM				((uint8_t[]){27,91,66,0})
#define KEY_RIGHT_ARROW					((uint8_t[]){27,91,67,0})
#define KEY_RIGHT_ARROW_NUM				((uint8_t[]){27,91,67,0})
#define KEY_LEFT_ARROW					((uint8_t[]){27,91,68,0})
#define KEY_LEFT_ARROW_NUM				((uint8_t[]){27,91,68,0})
#define KEY_5_NUM						((uint8_t[]){27,91,69,0})
#define KEY_END							((uint8_t[]){27,91,70,0})
#define KEY_END_NUM						((uint8_t[]){27,91,70,0})
#define KEY_HOME						((uint8_t[]){27,91,72,0})
#define KEY_HOME_NUM					((uint8_t[]){27,91,72,0})
#define KEY_SPACE						((uint8_t[]){32,0})
#define KEY_EXCLAMATION_MARK			((uint8_t[]){33,0})
#define KEY_HASHTAG						((uint8_t[]){35,0})
#define KEY_DOLLAR_SIGN					((uint8_t[]){36,0})
#define KEY_PERCENT_SIGN				((uint8_t[]){37,0})
#define KEY_AMPERSAND					((uint8_t[]){38,0})
#define KEY_APOSTROPHE					((uint8_t[]){39,0})
#define KEY_BRACKET_LEFT				((uint8_t[]){40,0})
#define KEY_BRACKET_RIGHT				((uint8_t[]){41,0})
#define KEY_ASTERISK					((uint8_t[]){42,0})
#define KEY_COMMA						((uint8_t[]){44,0})
#define KEY_DASH						((uint8_t[]){45,0})
#define KEY_PERIOD						((uint8_t[]){46,0})
#define KEY_SLASH						((uint8_t[]){47,0})
#define KEY_0							((uint8_t[]){48,0})
#define KEY_1							((uint8_t[]){49,0})
#define KEY_2							((uint8_t[]){50,0})
#define KEY_3							((uint8_t[]){51,0})
#define KEY_4							((uint8_t[]){52,0})
#define KEY_5							((uint8_t[]){53,0})
#define KEY_6							((uint8_t[]){54,0})
#define KEY_7							((uint8_t[]){55,0})
#define KEY_8							((uint8_t[]){56,0})
#define KEY_9							((uint8_t[]){57,0})
#define KEY_COLON						((uint8_t[]){58,0})
#define KEY_SEMICOLON					((uint8_t[]){59,0})
#define KEY_LESS_THAN_SIGN				((uint8_t[]){60,0})
#define KEY_EQUAL						((uint8_t[]){61,0})
#define KEY_MORE_THAN_SIGN				((uint8_t[]){62,0})
#define KEY_QUOTATION_MARK				((uint8_t[]){63,0})

#define KEY_AT_SIGN						((uint8_t[]){64,0})
#define KEY_SHIFT_A						((uint8_t[]){65,0})
#define KEY_SHIFT_B						((uint8_t[]){66,0})
#define KEY_SHIFT_C						((uint8_t[]){67,0})
#define KEY_SHIFT_D						((uint8_t[]){68,0})
#define KEY_SHIFT_E						((uint8_t[]){69,0})
#define KEY_SHIFT_F						((uint8_t[]){70,0})
#define KEY_SHIFT_G						((uint8_t[]){71,0})
#define KEY_SHIFT_H						((uint8_t[]){72,0})
#define KEY_SHIFT_I						((uint8_t[]){73,0})
#define KEY_SHIFT_J						((uint8_t[]){74,0})
#define KEY_SHIFT_K						((uint8_t[]){75,0})
#define KEY_SHIFT_L						((uint8_t[]){76,0})
#define KEY_SHIFT_M						((uint8_t[]){77,0})
#define KEY_SHIFT_N						((uint8_t[]){78,0})
#define KEY_SHIFT_O						((uint8_t[]){79,0})
#define KEY_SHIFT_P						((uint8_t[]){80,0})
#define KEY_SHIFT_Q						((uint8_t[]){81,0})
#define KEY_SHIFT_R						((uint8_t[]){82,0})
#define KEY_SHIFT_S						((uint8_t[]){83,0})
#define KEY_SHIFT_T						((uint8_t[]){84,0})
#define KEY_SHIFT_U						((uint8_t[]){85,0})
#define KEY_SHIFT_V						((uint8_t[]){86,0})
#define KEY_SHIFT_W						((uint8_t[]){87,0})
#define KEY_SHIFT_X						((uint8_t[]){88,0})
#define KEY_SHIFT_Y						((uint8_t[]){89,0})
#define KEY_SHIFT_Z						((uint8_t[]){90,0})
#define KEY_SQUARE_BRACKET_LEFT			((uint8_t[]){91,0})
#define KEY_BACKSLASH					((uint8_t[]){92,0})
#define KEY_SQUARE_BRACKET_RIGHT		((uint8_t[]){93,0})
#define KEY_CARET						((uint8_t[]){94,0})
#define KEY_UNDERSCORE					((uint8_t[]){95,0})
#define KEY_BACKTICK					((uint8_t[]){96,0})

#define KEY_A							((uint8_t[]){97,0})
#define KEY_B							((uint8_t[]){98,0})
#define KEY_C							((uint8_t[]){99,0})
#define KEY_D							((uint8_t[]){100,0})
#define KEY_E							((uint8_t[]){101,0})
#define KEY_F							((uint8_t[]){102,0})
#define KEY_G							((uint8_t[]){103,0})
#define KEY_H							((uint8_t[]){104,0})
#define KEY_I							((uint8_t[]){105,0})
#define KEY_J							((uint8_t[]){106,0})
#define KEY_K							((uint8_t[]){107,0})
#define KEY_L							((uint8_t[]){108,0})
#define KEY_M							((uint8_t[]){109,0})
#define KEY_N							((uint8_t[]){110,0})
#define KEY_O							((uint8_t[]){111,0})
#define KEY_P							((uint8_t[]){112,0})
#define KEY_Q							((uint8_t[]){113,0})
#define KEY_R							((uint8_t[]){114,0})
#define KEY_S							((uint8_t[]){115,0})
#define KEY_T							((uint8_t[]){116,0})
#define KEY_U							((uint8_t[]){117,0})
#define KEY_V							((uint8_t[]){118,0})
#define KEY_W							((uint8_t[]){119,0})
#define KEY_X							((uint8_t[]){120,0})
#define KEY_Y							((uint8_t[]){121,0})
#define KEY_Z							((uint8_t[]){122,0})

#define KEY_CURLY_BRACKET_LEFT			((uint8_t[]){123,0})
#define KEY_VERTICAL_BAR				((uint8_t[]){124,0})
#define KEY_CURLY_BRACKET_RIGHT			((uint8_t[]){125,0})
#define KEY_TILDE						((uint8_t[]){126,0})
#define KEY_BACKSPACE					((uint8_t[]){127,0})
#define KEY_POUND_SIGN					((uint8_t[]){194,163,0})
#define KEY_CURRENCY_SIGN				((uint8_t[]){194,164,0})
#define KEY_SECTION_SIGN				((uint8_t[]){194,167,0})
#define KEY_FRONTTICK					((uint8_t[]){194,180,0})
#define KEY_ONE_HALF					((uint8_t[]){194,189,0})

#endif