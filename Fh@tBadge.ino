// Timer 3 setup
#define TICKS_PER_SECOND 40000000	// This value works for Flinduino, at 40 MHz ?
#define T3_ON 0x8000				// Turn on Timer 3
#define T3_PS_1_1 0					// Set Prescale to 1:64????
#define T3_SOURCE_INT 0				//  Timer source internal timer ???

#define ON 			1
#define OFF 		0
#define COL_ON		HIGH	// Columns are Anodes
#define COL_OFF 	LOW
#define ROW_ON		LOW		// Rows are Cathodes
#define ROW_OFF		HIGH
const int numCol = 8;		// The display has 8 columns
const int numRow = 8;		// and 8 rows

// FH@T Badge display with black back
//const int colPin[numCol] = { 8, 13,7,11,0,6,1,4 };		// Thes are the pins that the columns are connected to in order from col 0
//const int rowPin[numRow] = { 15,14,10,5,9,3,2,12 };		// Thes are the pins that the rows are connected to in order from row 0

// FH@T Badge display with white back
const int colPin[numCol] = { 4, 1,6,0,11,7,13,8 };			// Thes are the pins that the columns are connected to in order from col 0
const int rowPin[numRow] = { 12,2,3,9,5,10,14,15 };			// Thes are the pins that the rows are connected to in order from row 0

// Tony's display
//const int colPin[numCol] = { 0, 1,2,3,4,5,6,7 };			// Thes are the pins that the columns are connected to in order from col 0
//const int rowPin[numRow] = { 8,9,10,11,12,22,23,24 };		// Thes are the pins that the rows are connected to in order from row 0

// Display configuration
#define MIRROR_ROW
//#define MIRROR_COL
//#define TRANSPOSE

int colIdx = 0;				// Index of column to be refreshed

int display[8];				// This array holds the current image we want to display
int invertDisplay = 0;

// Include animation converted from GIF
#include "anim.h"

// These are some different screens
static const uint8_t PROGMEM smile_bmp[] = {
	B00111100,
	B01000010,
	B10100101,
	B10000001,
	B10100101,
	B10011001,
	B01000010,
	B00111100
};

static const uint8_t PROGMEM neutral_bmp[] = {
	B00111100,
	B01000010,
	B10100101,
	B10000001,
	B10111101,
	B10000001,
	B01000010,
	B00111100
};

static const uint8_t PROGMEM frown_bmp[] = {
	B00111100,
	B01000010,
	B10100101,
	B10000001,
	B10011001,
	B10100101,
	B01000010,
	B00111100
};

static const uint8_t PROGMEM heart[] = {
	B00000000,
	B01100110,
	B10011001,
	B10000001,
	B01000010,
	B00100100,
	B00011000,
	B00000000
};
// Intro letters (slightly bigger)
static const uint8_t PROGMEM splash_F[] = { 0x7E, 0x7E,	0x60, 0x7C,0x7C,0x60, 0x60, 0x60 };
static const uint8_t PROGMEM  splash_H[] = { 0x66, 0x66, 0x66, 0x7E, 0x7E, 0x66, 0x66, 0x66 };
static const uint8_t PROGMEM splash_at[] = { 0x3C, 0x66, 0xDB, 0xA9, 0xAD, 0xDE, 0x60, 0x3C };
static const uint8_t PROGMEM splash_T[] = { 0x7E, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 };

// The character set courtesy of cosmicvoid.
// Ascii starting at dec 32 or "Blank"
static const uint8_t PROGMEM Font8x5[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Blank
	0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x00, // !
	0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,	//  "
	0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A, 0x00,	// #
	0x0E, 0x15, 0x05, 0x0E, 0x14, 0x15, 0x0E, 0x00,	// $
	0x13, 0x13, 0x08, 0x04, 0x02, 0x19, 0x19, 0x00,	// %
	0x06, 0x09, 0x05, 0x02, 0x15, 0x09, 0x16, 0x00,	// &
	0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,	// '
	0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x04, 0x00,	// (
	0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01, 0x00,	// )
	0x00, 0x0A, 0x15, 0x0E, 0x15, 0x0A, 0x00, 0x00,	// *
	0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x00,	// +
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x01,	// '
	0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,	// -
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,	// .
	0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01, 0x00,	// /
	0x0E, 0x11, 0x19, 0x15, 0x13, 0x11, 0x0E, 0x00,	// 0
	0x04, 0x06, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,	// 1
	0x0E, 0x11, 0x10, 0x0C, 0x02, 0x01, 0x1F, 0x00,	// 2
	0x0E, 0x11, 0x10, 0x0C, 0x10, 0x11, 0x0E, 0x00,	// 3
	0x08, 0x0C, 0x0A, 0x09, 0x1F, 0x08, 0x08, 0x00,	// 4
	0x1F, 0x01, 0x01, 0x0E, 0x10, 0x11, 0x0E, 0x00,	// 5
	0x0C, 0x02, 0x01, 0x0F, 0x11, 0x11, 0x0E, 0x00,	// 6
	0x1F, 0x10, 0x08, 0x04, 0x02, 0x02, 0x02, 0x00,	// 7
	0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x00,	// 8
	0x0E, 0x11, 0x11, 0x1E, 0x10, 0x08, 0x06, 0x00,	// 9
	0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,	// :
	0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x02, 0x01,	// ;
	0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00,	// <
	0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00,	// =
	0x01, 0x02, 0x04, 0x08, 0x04, 0x02, 0x01, 0x00,	// >
	0x0E, 0x11, 0x10, 0x08, 0x04, 0x00, 0x04, 0x00,	// ?
	0x0E, 0x11, 0x1D, 0x15, 0x0D, 0x01, 0x1E, 0x00,	// @
	0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00,	// A
	0x0F, 0x11, 0x11, 0x0F, 0x11, 0x11, 0x0F, 0x00,	// B
	0x0E, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0E, 0x00,	// C
	0x07, 0x09, 0x11, 0x11, 0x11, 0x09, 0x07, 0x00,	// D
	0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x1F, 0x00,	// E
	0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x01, 0x00,	// F
	0x0E, 0x11, 0x01, 0x0D, 0x11, 0x19, 0x16, 0x00,	// G
	0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00,	// H
	0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x07, 0x00,	// I
	0x1C, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06, 0x00,	// J
	0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11, 0x00,	// K
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0F, 0x00,	// L
	0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00,	// M
	0x11, 0x13, 0x13, 0x15, 0x19, 0x19, 0x11, 0x00,	// N
	0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,	// O
	0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00,	// P
	0x0E, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16, 0x00,	// Q
	0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00,	// R
	0x0E, 0x11, 0x01, 0x0E, 0x10, 0x11, 0x0E, 0x00,	// S
	0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,	// T
	0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,	// U
	0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04, 0x00,	// V
	0x41, 0x41, 0x41, 0x49, 0x2A, 0x2A, 0x14, 0x00,	// W
	0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 0x00,	// X
	0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00,	// Y
	0x1F, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1F, 0x00,	// Z
	0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07, 0x00,	// [
	0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00,	// \
	0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x00,	// ]
	0x00, 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00,	// ^
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00,	// _
	0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,	// `
	0x00, 0x00, 0x06, 0x08, 0x0E, 0x09, 0x0E, 0x00,	// a
	0x01, 0x01, 0x0D, 0x13, 0x11, 0x13, 0x0D, 0x00,	// b
	0x00, 0x00, 0x06, 0x09, 0x01, 0x09, 0x06, 0x00,	// c
	0x10, 0x10, 0x16, 0x19, 0x11, 0x19, 0x16, 0x00,	// d
	0x00, 0x00, 0x06, 0x09, 0x07, 0x01, 0x0E, 0x00,	// e
	0x04, 0x0A, 0x02, 0x07, 0x02, 0x02, 0x02, 0x00,	// f
	0x00, 0x00, 0x06, 0x09, 0x09, 0x06, 0x08, 0x07,	// g
	0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,	// h
	0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00,	// i
	0x04, 0x00, 0x06, 0x04, 0x04, 0x04, 0x04, 0x03,	// j
	0x01, 0x01, 0x09, 0x05, 0x03, 0x05, 0x09, 0x00,	// k
	0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,	// l
	0x00, 0x00, 0x15, 0x2B, 0x29, 0x29, 0x29, 0x00,	// m
	0x00, 0x00, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,	// n
	0x00, 0x00, 0x06, 0x09, 0x09, 0x09, 0x06, 0x00,	// o
	0x00, 0x00, 0x0D, 0x13, 0x13, 0x0D, 0x01, 0x01,	// p
	0x00, 0x00, 0x16, 0x19, 0x19, 0x16, 0x10, 0x10,	// q
	0x00, 0x00, 0x0D, 0x13, 0x01, 0x01, 0x01, 0x00,	// r
	0x00, 0x00, 0x0E, 0x01, 0x06, 0x08, 0x07, 0x00,	// s
	0x00, 0x02, 0x07, 0x02, 0x02, 0x02, 0x04, 0x00,	// t
	0x00, 0x00, 0x11, 0x11, 0x11, 0x19, 0x16, 0x00,	// u
	0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00,	// v
	0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00,	// w
	0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00,	// x
	0x00, 0x00, 0x09, 0x09, 0x09, 0x0E, 0x08, 0x06,	// y
	0x00, 0x00, 0x0F, 0x08, 0x06, 0x01, 0x0F, 0x00,	// z
	0x04, 0x02, 0x02, 0x01, 0x02, 0x02, 0x04, 0x00,	// {
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,	// |
	0x01, 0x02, 0x02, 0x04, 0x02, 0x02, 0x01, 0x00,	// }
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// ~
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// del
	0x1C, 0x2A, 0x49, 0x49, 0x41, 0x22, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x51, 0x49, 0x41, 0x22, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x41, 0x79, 0x41, 0x22, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x41, 0x49, 0x51, 0x22, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x41, 0x49, 0x49, 0x2A, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x41, 0x49, 0x45, 0x22, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x41, 0x4F, 0x41, 0x22, 0x1C, 0x00,	// clock??
	0x1C, 0x22, 0x45, 0x49, 0x41, 0x22, 0x1C, 0x00, // clock??
};

static const uint8_t PROGMEM lentbl_S[104] = {
	2, 2, 3, 5, 5, 5, 5, 2,
	3, 3, 5, 5, 2, 5, 1, 5,
	5, 4, 5, 5, 5, 5, 5, 5,
	5, 5, 1, 2, 4, 4, 4, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 3, 5, 5, 4, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 7,
	5, 5, 5, 3, 5, 3, 5, 5,
	2, 4, 5, 4, 5, 4, 4, 4,
	5, 2, 3, 4, 2, 6, 5, 4,
	5, 5, 5, 4, 3, 5, 5, 5,
	5, 4, 4, 3, 2, 3, 0, 0,
	7, 7, 7, 7, 7, 7, 7, 7
};

/**********************************************************************
 ************************* HELPER ROUTINES ****************************
 **********************************************************************/
// Define the Interrupt Service Routine (ISR)
// For the timer
static void __attribute__((interrupt)) myISR() {
	RefreshDisplay();

	colIdx = (colIdx + 1) % numCol;

	clearIntFlag(_TIMER_3_IRQ);
}


// start_timer_3 for the timer interupt
static void start_timer_3(uint32_t frequency) {
	uint32_t period;
	period = TICKS_PER_SECOND / frequency;
	T3CONCLR = T3_ON;			// Turn the timer off
	T3CON = T3_PS_1_1;			// Set the prescaler
	TMR3 = 0;					// Clear the counter
	PR3 = period;				// Set the period
	T3CONSET = T3_ON;			// Turn the timer on
}

// Refresh part of the display
// Draws a row at once but is called so frequently you can't tell
static void RefreshDisplay() {
	bool rowState = ROW_ON;
	int c, r, rowPtr, tmp;

	// De-select old column
	for (int c = 0; c < numCol; c++)
		digitalWrite(colPin[c], COL_OFF);

	// De-select old rows
	for (int r = 0; r < numRow; r++)
		digitalWrite(rowPin[r], ROW_OFF);

	// Select current column
	digitalWrite(colPin[colIdx], COL_ON);

	if (invertDisplay)
		rowState = ROW_OFF;

	c = colIdx;
#ifdef MIRROR_COL
	c = 7 - c;
#endif
	// Light up each row as necessary
	for (rowPtr = 0; rowPtr < numRow; rowPtr++) {
		r = rowPtr;
#ifdef MIRROR_ROW
		r = 7 - r;
#endif
#ifdef TRANSPOSE
		r = r ^ c;
		c = r ^ c;
		r = r ^ c;
#endif
		if (((display[r] >> c) & 1) == 1)
			digitalWrite(rowPin[rowPtr], rowState);		// Check if the DOT needs to be on
		else
			digitalWrite(rowPin[rowPtr], !rowState);	// other wise turn it off
	}
}

// Flip bit order of a byte (for the font which is stored backwards..)
static uint8_t revByte(uint8_t b) {
		b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
		b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
		b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
		return b;
}

// Look up font data for a single character
static const uint8_t *getFontData(char c) {
	int ofs = c - ' ';
	if (ofs > sizeof(Font8x5) / 8) // Truncate to space
		ofs = 0;
	return Font8x5 + ofs * 8;
}

static const int getCharLen(char c) {
	int ofs = c - ' ';
	if (ofs > sizeof(Font8x5) / 8) // Truncate to space
		ofs = 0;
	return lentbl_S[ofs];
}

/**********************************************************************
 ************************ DISPLAY ROUTINES ****************************
 **********************************************************************/
// clears the display, All LEDs off
void ClearDisplay(void) {
	for (int i = 0; i < numCol; i++)
		display[i] = 0;
}

// Dots all on
void FillDisplay(void)
{
	for (int i = 0; i < numCol; i++)
		display[i] = 0xff;
}

// Display a single character on the screen
void DisplayChar(char c) {
	const uint8_t *d = getFontData(c);
	for (int i = 0; i < 8; i++)
		display[i] = revByte(d[i]);
}

// Displays an 8 x 8 splash screen
void DisplaySplash(const uint8_t splash[numCol]) {
	for (int i = 0; i < numCol; i++)
		display[i] = splash[i];
}

// Draws a single pixel at coordinate x, y 
// if state is 1 LED will be turned on
// if state is 0 LED will be off
void DrawPixel( int x, int y, int state) {
	if ((y < 0) || (y >= numCol))
		return;
	if ((x < 0) || (x >= numRow))
		return;

	if (state)
		display[y] |= 1 << x;
	else
		display[y] &= ~(1 << x);
}

// matrix waterfall display
void DisplayMatrix(int time, int dtime) {
	for (int t = 0; t < time * 1000 / dtime; t++) {
		for (int i = 6; i >= 0; i--)
			display[i + 1] = display[i];
		delay(dtime);
		display[0] = byte(random(256));
	}
}

// Scroll text across the display
void DisplayText(const char PROGMEM *msg, int coldelay) {
	const int len = strlen(msg);
	const uint8_t *fdata;
	int charwidth = 0, charpos = 0, lastchar = 0;

	while (1) {
		// Do we need to look for a new character?
		if (charpos == charwidth) {
			// Is this the end of the message?
			if (*msg != '\0') {
				charwidth = getCharLen(*msg) + 1; // Add one to get a gap between letters
				fdata = getFontData(*msg);
				msg++;
			} else {
				// If we are at the end of the message just push blank columns along
				charwidth = 8;
				fdata = getFontData(' ');
				lastchar = 1;
			}
			charpos = 0;
		}

		// Scroll left everything in the display
		for (int r = 0; r < numRow; r++) {
			display[r] = display[r] << 1;
		}

		// Add new data in
		for (int r = 0; r < numRow; r++) {
			display[r] |= (fdata[r] >> charpos) & 0x01;
		}

		// Move to next column for this character
		charpos++;

		// End of message?
		if (lastchar && charpos == charwidth)
			break;

		delay(coldelay);
	}
}

// Show animation
void DisplayAnim(const uint8_t PROGMEM *anim, const uint8_t PROGMEM *durs, int nframes) {
	for (int i = 0; i < nframes; i++) {
		for (int r = 0; r < 8; r++)
			display[r] = anim[i * 8 + r];
		delay(durs[i]);
	}
}

/* Arduino setup function, called once */
void setup() {
	invertDisplay = false;
	// Set all column pins to outputs and off
	for (int i = 0; i < numCol; i++) {
		pinMode(colPin[i], OUTPUT);
		digitalWrite(colPin[i], COL_OFF);
	}
	// Set all row pins to outputs and off
	for (int i = 0; i < numCol; i++) {
		pinMode(rowPin[i], OUTPUT);
		digitalWrite(rowPin[i], ROW_OFF);
	}

	// Start the timer & install ISR
	start_timer_3(5);	// 1 kHz
	setIntVector(_TIMER_3_VECTOR, myISR);
	setIntPriority(_TIMER_3_VECTOR, 4, 0);
	clearIntFlag(_TIMER_3_IRQ);
	setIntEnable(_TIMER_3_IRQ);

	//Start the serial Port
	Serial.begin(9600);

	DisplaySplash(splash_F);
	delay(400);
	DisplaySplash(splash_H);
	delay(400);
	DisplaySplash(splash_at);
	delay(400);
	DisplaySplash(splash_T);
	delay(400);
	ClearDisplay();
	delay(2000);

	Serial.println("FH@T");
}

/* Arduino loop function, called over and over */
void loop() {
	const char PROGMEM *msg = "Flinders & Hackerspace @ Tonsley";

	// Display Matrix for 10 seconds at 8 FPS
	ClearDisplay();
	DisplayMatrix(10, 125);

	// Display animation
	ClearDisplay();
	delay(500);
	DisplayAnim(anim, anim_durs, sizeof(anim) / 8);

	// Display scrolling message (75 msec per column scrolled)
	ClearDisplay();
	delay(500);
	DisplayText(msg, 75);

	// Flash Display
	for (int i = 0; i < 5; i++) {
		FillDisplay();
		delay(250);
		ClearDisplay();
		delay(250);
	}

	// Display some icons
	invertDisplay = false;
	ClearDisplay();
	DisplaySplash(frown_bmp);

	delay(1000);
	DisplaySplash(neutral_bmp);

	delay(1000);
	DisplaySplash(smile_bmp);

	delay(1000);
	DisplaySplash(heart);
	invertDisplay = true;

	ClearDisplay();
	DisplaySplash(frown_bmp);

	delay(1000);
	DisplaySplash(neutral_bmp);

	delay(1000);
	DisplaySplash(smile_bmp);

	delay(1000);
	DisplaySplash(heart);

	invertDisplay = true;

	// Light up pixels in sequence and back again
	delay(1000);
	ClearDisplay();
	for (int c = 0; c < 8; c++) {
		for (int r = 0; r < 8; r++) {
			DrawPixel(c, r, ON);
			delay(25);
		}
	}
	for (int c = 7; c >= 0; c--) {
		for (int r = 7; r >=0; r--) {
			DrawPixel(c, r, OFF);
			delay(25);
		}
	}

	for (int r = 0; r < 8; r++) {
		for (int c = 0; c < 8; c++) {
			DrawPixel( c, r, ON);
			delay(25);
		}
	}
	for (int r = 7; r >= 0; r--) {
		for (int c = 7; c >=0; c--) {
			DrawPixel( c, r, OFF);
			delay(25);
		}
	}
}



/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * mode: c
 * End:
 */
