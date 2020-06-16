/** An input dialog which reads an integer value from the user. The dialog accepts only digit
 ** characters. All non-digit characters, except 'd', are ignored and not even echoed on the screen.
 ** The 'd' character is used to delete the very last digit from the input, i.e. it plays the role 
 ** of BACKSPACE. The dialog is closed either when the user presses the RETURN key or the ESCAPE
 ** key. Pressing the RETURN key means that the user confirms his input. Pressing the ESCAPE key
 ** means that the user cancels his input. 
 **
 ** The dialog occupies two lines. The top line contains the prompt, while the bottom line is
 ** where the user types his input.
 **
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <termios.h>

/// max length of an input buffer, i.e. the place
/// where the dialog temporarily stores the user's 
/// input data 
#define INPUT_BUFFER_MAX_LEN 11

/// this is what is displayed at the beginning of the dialog's
/// bottom line
#define INPUT_PREFIX "? "

/// to make the dialog work properly this mask must
/// be applied to the terminal's attributes
/// (see the function below)
#define NEW_TERM_ATTRIBS_MASK (~ICANON & ~ECHO & ~ISIG)

/// clears the screen's current line
#define clear_line()    printf("\33[2K\r")

/// clears the screen
#define clear_screen()  printf("\033[H\033[J")

/// sets the cursor position on the screen
#define goto_xy(x_,y_)  printf("\033[%d;%dH", (y_), (x_))

/// That's how the input dialog is invoked. The first two params
/// are the screen's coordinates for the dialog. The third param
/// is the dialog's prompt and the last param is the user's input value.
/// 
/// The function returns (1) if the user closes the dialog with the RETURN key, 
/// and (0) if the user closes the dialog with the ESCAPE key. 
static int32_t show_int_input_dialog(const int32_t x, 
                                      const int32_t y, 
                                      const char* prompt, 
                                      int32_t* input_ptr);

// makes an alert sound when the user tries to enter an illegal character
static void make_alert_sound(void);                                      

int main(int argc, char** argv)
{
	int32_t n;
	clear_screen();
	if (show_int_input_dialog(1, 1, "Enter your number: ", &n)) {
	  printf("Your number is %d\n", n);
	}
	return 0;
}

static int32_t show_int_input_dialog(const int32_t x, 
                                      const int32_t y, 
                                      const char* prompt, 
                                      int32_t* input_ptr)
{
	// return value
	int32_t ret_val = 1;
	
  // the input buffer (see comments above)
	char input_buffer[INPUT_BUFFER_MAX_LEN];
	memset(input_buffer, 0, sizeof(input_buffer));
	
	// before printing the dialog's top line (see comments above)
	// we want to clear it of all stray characters left on the screen
	goto_xy(x, y);
	clear_line();
	printf("%s", prompt);
	
	// similarly, before printing the dialog's bottom line (see comments above)
	// we also want to clear it of all stray characters left on the screen	
	goto_xy(x, y + 1);
	clear_line();
	printf("%s", INPUT_PREFIX);

	// the current number of digit characters in the input buffer
	int32_t n_input_chars = 0;
	
	// To enable the proper work of the dialog we want to temporaily change the terminal's attributes.
	// That is we want to disable canonical mode (buffered i/o) and local echo.
	// The new attributes will be set thru the new_tio variable. The old_tio variable
	// is used to store the terminal's previous attributes so that we can restore them
	// after the function finishes its work.
	struct termios old_tio, new_tio;
	tcgetattr(STDIN_FILENO,&old_tio);
	new_tio=old_tio;
	new_tio.c_lflag &= NEW_TERM_ATTRIBS_MASK;
	tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);	

  // placing the cursor to where the user has to type his input
	goto_xy(x + (int32_t) strlen(INPUT_PREFIX), y + 1);
	
	// a separate character typed by the user
	char c;
		
	// In this loop we process the characters typed by the user. 
	while (1) {
		c = getchar();
		
		// if the user presses ESCAPE
		if (c == '\033') {
		  ret_val = 0;
		  break;
		}
		
		// if the user presses RETURN
		if (c == '\n') {
		
			// if the input buffer is empty, the RETURN key is ignored
			if (!n_input_chars) {
				make_alert_sound();
			} else {
				break;
			}
		}
		
		// if the user presses a non-digit character
		if (!isdigit(c)) {
		  
		  // 'd' character is legal
		  if ('d' == c) {
		    if (n_input_chars) { // the input buffer is not empty
		      n_input_chars--;
		    } else { // the input buffer has nothing to delete
		      make_alert_sound();
		    }
		  } else { // other non-digit characters are illegal
		    make_alert_sound();
		  }
		} else {
		  if (n_input_chars < INPUT_BUFFER_MAX_LEN) { // if the input buffer has space for a new character
		    input_buffer[n_input_chars++] = c;
		  } else {
		    make_alert_sound();
		  }  
		}
 
    // redrawing the dialog's bottom line to reflect the last changes made by the user 
		goto_xy(x, y + 1);
		clear_line();		
    printf("%s", INPUT_PREFIX);		
	  goto_xy(x + (int32_t) strlen(INPUT_PREFIX), y + 1);
		printf("%.*s", n_input_chars, input_buffer);
	}
	
	// restoring the terminal's original attributes
	tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);

  if (ret_val) {
	  input_buffer[n_input_chars] = 0;
	  *input_ptr = atoi(input_buffer);
	} else {
  	goto_xy(x + (int32_t) strlen(INPUT_PREFIX), y + 1);
  	clear_line();
	  printf("Cancelled\n");
	  *input_ptr = 0;
	}
	printf("\n");
	return ret_val;
}

static void make_alert_sound(void)
{
  printf("\a");
}
