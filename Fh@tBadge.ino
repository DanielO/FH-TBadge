/* Final clock frequency for CPU is..
 * = Crystal frequency / FPLLIDIV * PLLMULT / PLLODIV
 * = 8 / 2 * 20 / 2
 * = 40MHz
 *
 * Frequency to peripherals is
 * = CPU clock / PBDIV
 * = 40 / 1
 * = 40MHz
 */
#define TICKS_PER_SECOND 40000000

#define COL_ON		HIGH			// Columns are Anodes
#define COL_OFF 	LOW
#define ROW_ON		LOW				// Rows are Cathodes
#define ROW_OFF		HIGH
#define NUM_COL		8
#define NUM_ROW		8

// List of display routines to run in the loop
#define DO_TEXT
#define DO_FIRE
#define DO_CONWAY
#define DO_MATRIX
#define DO_ANIM
#define DO_FLASH
#define DO_ICONS
#define DO_SEQUENCE

// FH@T Badge display with black back
//const int colPin[NUM_COL] = { 8, 13,7,11,0,6,1,4 };		// Thes are the pins that the columns are connected to in order from col 0
//const int rowPin[NUM_ROW] = { 15,14,10,5,9,3,2,12 };		// Thes are the pins that the rows are connected to in order from row 0

// FH@T Badge display with white back
const int colPin[NUM_COL] = { 4, 1,6,0,11,7,13,8 };			// Thes are the pins that the columns are connected to in order from col 0
const int rowPin[NUM_ROW] = { 12,2,3,9,5,10,14,15 };			// Thes are the pins that the rows are connected to in order from row 0

// Tony's display
//const int colPin[NUM_COL] = { 0, 1,2,3,4,5,6,7 };			// Thes are the pins that the columns are connected to in order from col 0
//const int rowPin[NUM_ROW] = { 8,9,10,11,12,22,23,24 };		// Thes are the pins that the rows are connected to in order from row 0

// Display configuration
#define MIRROR_ROW
#define MIRROR_COL
//#define TRANSPOSE

int colIdx = 0;						// Index of column to be refreshed
uint8_t display[NUM_ROW][NUM_COL];	// This array holds the current image we want to display
int invertDisplay = 0;
int brightnessCount = 0;

// Brightness levels
uint8_t brights[] = {0, 2, 5, 7, 9, 13, 21, 31};
#define NUM_BRIGHTS (sizeof(brights) / sizeof(brights[0]))
#define MIN_BRIGHT (0)
#define MAX_BRIGHT (NUM_BRIGHTS - 1)

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
static const uint8_t PROGMEM splash_F[] = {
	B01111110,
	B01111110,
	B01100000,
	B01111100,
	B01111100,
	B01100000,
	B01100000,
	B01100000,
};

static const uint8_t PROGMEM  splash_H[] = {
	B01100110,
	B01100110,
	B01100110,
	B01111110,
	B01111110,
	B01100110,
	B01100110,
	B01100110,
};

static const uint8_t PROGMEM splash_at[] = {
	B00111100,
	B01100110,
	B11011011,
	B10101001,
	B10101101,
	B11011110,
	B01100000,
	B00111100,
};

static const uint8_t PROGMEM splash_T[] = {
	B01111110,
	B01111110,
	B00011000,
	B00011000,
	B00011000,
	B00011000,
	B00011000,
	B00011000,
};

#include "font.h"

// Conway Life boards
static const uint8_t board1[NUM_ROW][NUM_COL] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0 }
};

/**********************************************************************
 ************************* HELPER ROUTINES ****************************
 **********************************************************************/
// Define the Interrupt Service Routine (ISR)
// For the timer
static void __attribute__((interrupt)) myISR() {
	RefreshDisplay();

	clearIntFlag(_TIMER_3_IRQ);
}

// start_timer_3 for the timer interupt
static int start_timer_3(uint32_t frequency) {
	uint32_t period;

	period = TICKS_PER_SECOND / frequency;
	if (period < 1 || period > 65535)
		return 1;
	T3CONCLR = T3_ON;			// Turn the timer off
	T3CON = T3_PS_1_1;			// Set the prescaler
	TMR3 = 0;					// Clear the counter
	PR3 = period;				// Set the period
	T3CONSET = T3_ON;			// Turn the timer on

	return 0;
}

// Refresh part of the display
// Draws a column at once but is called so frequently you can't tell
static void RefreshDisplay() {
	bool rowState = ROW_ON;
	int c, r, rowPtr;

	// De-select old column
	for (int c = 0; c < NUM_COL; c++)
		digitalWrite(colPin[c], COL_OFF);

	// De-select old rows
	for (int r = 0; r < NUM_ROW; r++)
		digitalWrite(rowPin[r], ROW_OFF);

	brightnessCount = (brightnessCount + 1) % brights[MAX_BRIGHT];
	// Only move to the next column when we've done all the brightness stuff for this one
	if (brightnessCount == 0)
		colIdx = (colIdx + 1) % NUM_COL;

	// Select current column
	digitalWrite(colPin[colIdx], COL_ON);

	if (invertDisplay)
		rowState = ROW_OFF;

	c = colIdx;
#ifdef MIRROR_COL
	c = 7 - c;
#endif
	// Light up each row as necessary
	for (rowPtr = 0; rowPtr < NUM_ROW; rowPtr++) {
		r = rowPtr;
#ifdef MIRROR_ROW
		r = 7 - r;
#endif
#ifdef TRANSPOSE
		r = r ^ c;
		c = r ^ c;
		r = r ^ c;
#endif
		if (display[r][c] != 0 && brights[display[r][c]] >= brightnessCount)
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
	if (ofs > (int)(sizeof(Font8x5) / 8)) // Truncate to space
		ofs = 0;
	return Font8x5 + ofs * 8;
}

static const int getCharLen(char c) {
	int ofs = c - ' ';
	if (ofs > (int)(sizeof(Font8x5) / 8)) // Truncate to space
		ofs = 0;
	return lentbl_S[ofs];
}

int PctChance(int chance) {
	if (chance < 0)
		chance = 0;
	if (chance > 100)
		chance = 100;
	if (chance > byte(random(100)))
		return 1;
	else
		return 0;
}
/**********************************************************************
 ************************ DISPLAY ROUTINES ****************************
 **********************************************************************/
// clears the display, All LEDs off
void ClearDisplay(void) {
	for (int r = 0; r < NUM_ROW; r++)
		for (int c = 0; c < NUM_COL; c++)
			display[r][c] = MIN_BRIGHT;
}

// Dots all on
void FillDisplay(void)
{
	for (int r = 0; r < NUM_ROW; r++)
		for (int c = 0; c < NUM_COL; c++)
			display[r][c] = MAX_BRIGHT;
}

// Display a single character on the screen
void DisplayChar(char c) {
	const uint8_t *d = getFontData(c);

	for (int r = 0; r < NUM_ROW; r++) {
		const uint8_t tmp = revByte(d[r]);
		for (int c = 0; c < NUM_COL; c++)
			display[r][c] = tmp >> (NUM_COL - 1 - c) & 0x1 ? MAX_BRIGHT : MIN_BRIGHT;
	}
}

// Displays an 8 x 8 splash screen
void DisplaySplash(const uint8_t splash[NUM_COL], int brightness) {
	for (int r = 0; r < NUM_ROW; r++)
		for (int c = 0; c < NUM_COL; c++)
			display[r][c] = splash[r] & (1 << (NUM_COL - 1 - c)) ? brightness : MIN_BRIGHT;
}

// Fade down the brightness over time
void FadeDown(const uint8_t splash[NUM_COL], int dly) {
	for (int b = NUM_BRIGHTS - 1; b >= 0; b--) {
		DisplaySplash(splash, b);
		delay(dly);
	}
	delay(dly * 2);
}

// Draws a single pixel at coordinate row, col
void DrawPixel(int c, int r, int brightness) {
	if (c < 0 || c >= NUM_COL)
		return;
	if (r < 0 || r >= NUM_ROW)
		return;

	display[r][c] = brightness;
}

// matrix waterfall display
void DisplayMatrix(int time, int dtime) {
	for (int t = 0; t < time * 1000 / dtime; t++) {
		for (int r = NUM_ROW - 2; r >= 0; r--)
			for (int c = 0; c < NUM_COL; c++) {
				if (display[r][c] == 0)
					display[r + 1][c] = 0;
				else
					display[r + 1][c] = display[r][c] - random(2) * 2;
			}
		delay(dtime);

		uint8_t rnd = byte(random(256));
		for (int c = 0; c < 8; c++)
			display[0][c] = rnd & 1 << c ? MAX_BRIGHT : MIN_BRIGHT;
	}
}

// Fire
// random(fadeamt - 1) will be subtracted each iteration
// seedchance is out of 100
void DisplayFire(int time, int dtime, int fadeamt, int seedchance) {
	for (int t = 0; t < time * 1000 / dtime; t++) {
		// Move fire up and fade
		for (int r = 0; r < NUM_ROW; r++) {
			for (int c = 0; c < NUM_COL; c++) {
				uint8_t rnd = byte(random(fadeamt));
				uint8_t oldval = display[r + 1][c];
				if (oldval == MIN_BRIGHT || oldval <= rnd)
					display[r][c] = MIN_BRIGHT; // Old value is already off or will be due to fading
				else
					display[r][c] = oldval - rnd; // Fade the old value
			}
		}
		// Seed new fire
		for (int c = 0; c < NUM_COL; c++)
			display[NUM_ROW - 1][c] = 0;
		for (int c = 0; c < NUM_COL; c++) {
			int bias = 0;
			// Bias seedchance to make 2 "hot spots"
			if (c == 1 || c == 2)
				bias = 30;
			if (c == 5 || c == 6)
				bias = 30;
			if (PctChance(seedchance + bias)) {
				if (c > 1)
					display[NUM_ROW - 1][c - 1] += MAX_BRIGHT - 3;
				display[NUM_ROW - 1][c] += MAX_BRIGHT;
				if (c < 7)
					display[NUM_ROW - 1][c + 1] += MAX_BRIGHT - 3;
			}
		}
		// Clip to maximum brightnes
		for (int c = 0; c < NUM_COL; c++)
			if (display[NUM_ROW - 1][c] > MAX_BRIGHT)
				display[NUM_ROW - 1][c] = MAX_BRIGHT;
		delay(dtime);
	}
}

// Scroll text across the display
void DisplayText(const char PROGMEM *msg, int coldelay) {
	const uint8_t *fdata = NULL;
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

		// Scroll everything left in the display
		for (int r = 0; r < NUM_ROW; r++)
			for (int c = 0; c < NUM_COL - 1;  c++)
				display[r][c] = display[r][c + 1];

		// Add new data in on right most column
		for (int r = 0; r < NUM_ROW; r++)
			display[r][NUM_COL - 1] = (fdata[r] >> charpos) & 0x01 ? MAX_BRIGHT : MIN_BRIGHT;

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
			for (int c = 0; c < 8; c++)
				display[r][c] = anim[i * 8 + r] & 1 << c ? MAX_BRIGHT : MIN_BRIGHT;
		delay(durs[i]);
	}
}

// Run Conway's game of life
// http://brownsofa.org/blog/2010/12/30/arduino-8x8-game-of-life/
/**
 * Counts the number of active cells surrounding the specified cell.
 * Cells outside the board are considered "off"
 * Returns a number in the range of 0 <= n < 9
 */
uint8_t countNeighbors(uint8_t board[NUM_ROW][NUM_COL], uint8_t row, uint8_t col) {
	uint8_t count = 0;
	for (char rowDelta = -1; rowDelta <= 1; rowDelta++) {
		for (char colDelta = -1; colDelta <= 1; colDelta++) {
			// skip the center cell
			if (!(colDelta == 0 && rowDelta == 0)) {
				if (isCellAlive(board, rowDelta + row, colDelta + col)) {
					count++;
				}
			}
		}
	}
	return count;
}

/**
 * Returns whether or not the specified cell is on.
 * If the cell specified is outside the game board, returns false.
 */
boolean isCellAlive(uint8_t board[NUM_ROW][NUM_COL], uint8_t row, uint8_t col) {
	if (row < 0 || col < 0 || row >= NUM_ROW || col >= NUM_COL) {
		return false;
	}

	return (board[row][col] == 1);
}

/**
 * Encodes the core rules of Conway's Game Of Life, and generates the next iteration of the board.
 * Rules taken from wikipedia.
 */
void calculateNewGameBoard(uint8_t oldboard[NUM_ROW][NUM_COL], uint8_t newboard[NUM_ROW][NUM_COL]) {
	for (uint8_t row = 0; row < NUM_ROW; row++) {
		for (uint8_t col = 0; col < NUM_COL; col++) {
			uint8_t numNeighbors = countNeighbors(oldboard, row, col);

			if (oldboard[row][col] && numNeighbors < 2) {
				// Any live cell with fewer than two live neighbours dies, as if caused by under-population.
				newboard[row][col] = false;
			} else if (oldboard[row][col] && (numNeighbors == 2 || numNeighbors == 3)) {
				// Any live cell with two or three live neighbours lives on to the next generation.
				newboard[row][col] = true;
			} else if (oldboard[row][col] && numNeighbors > 3) {
				// Any live cell with more than three live neighbours dies, as if by overcrowding.
				newboard[row][col] = false;
			} else if (!oldboard[row][col] && numNeighbors == 3) {
				// Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
				newboard[row][col] = true;
			} else {
				// All other cells will remain off
				newboard[row][col] = false;
			}
		}
	}
}

// Copy newboard to oldboard
void swapGameBoards(uint8_t oldboard[NUM_ROW][NUM_COL], uint8_t newboard[NUM_ROW][NUM_COL]) {
	for (uint8_t row = 0; row < NUM_ROW; row++) {
		for (uint8_t col = 0; col < NUM_COL; col++) {
			oldboard[row][col] = newboard[row][col];
		}
	}
}

void DisplayConway(const uint8_t PROGMEM start[NUM_ROW][NUM_COL], int time, int dtime) {
	uint8_t oldboard[NUM_ROW][NUM_COL] = { };
	uint8_t newboard[NUM_ROW][NUM_COL] = { };

	memcpy(oldboard, start, sizeof(oldboard));

	for (int t = 0; t < time * 1000 / dtime; t++) {
		for (uint8_t row = 0; row < NUM_ROW; row++) {
			for (uint8_t col = 0; col < NUM_COL; col++) {
				if (oldboard[row][col])
					display[row][col] = MAX_BRIGHT;
				else
					display[row][col] = MIN_BRIGHT;
			}
		}
		delay(dtime);
		calculateNewGameBoard(oldboard, newboard);
		swapGameBoards(oldboard, newboard);
	}
}

/* Arduino setup function, called once */
void setup() {
	invertDisplay = false;
	// Set all column pins to outputs and off
	for (int i = 0; i < NUM_COL; i++) {
		pinMode(colPin[i], OUTPUT);
		digitalWrite(colPin[i], COL_OFF);
	}
	// Set all row pins to outputs and off
	for (int i = 0; i < NUM_COL; i++) {
		pinMode(rowPin[i], OUTPUT);
		digitalWrite(rowPin[i], ROW_OFF);
	}

	// Start the timer & install ISR
	// For a 40MHz peripheral clock the frequency range is 610Hz - 40MHz
	if (start_timer_3(32000)) {	// Hz
		while (1)
			Serial.println("Unable to start timer");
	}
	setIntVector(_TIMER_3_VECTOR, myISR);
	setIntPriority(_TIMER_3_VECTOR, 4, 0);
	clearIntFlag(_TIMER_3_IRQ);
	setIntEnable(_TIMER_3_IRQ);

	//Start the serial Port
	Serial.begin(9600);

	FadeDown(splash_F, 50);
	FadeDown(splash_H, 50);
	FadeDown(splash_at, 50);
	FadeDown(splash_T, 50);

	ClearDisplay();
	delay(1000);

	Serial.println("FH@T");
}

/* Arduino loop function, called over and over */
void loop() {
	// Display scrolling message (75 msec per column scrolled)
#ifdef DO_TEXT
	const char PROGMEM *msg = "Flinders & Hackerspace @ Tonsley";

	ClearDisplay();
	delay(500);
	DisplayText(msg, 75);
#endif

	// Display Fire for 10 seconds at ~15 FPS
#ifdef DO_FIRE
	ClearDisplay();
	DisplayFire(10, 67, 4, 40);
	ClearDisplay();
#endif

	// Display Conway's game of life for 8 seconds at 8 FPS
#ifdef DO_CONWAY
	delay(500);
	ClearDisplay();
	DisplayConway(board1, 8, 125);
#endif

	// Display Matrix for 10 seconds at 8 FPS
#ifdef DO_MATRIX
	ClearDisplay();
	DisplayMatrix(10, 125);
#endif

	// Display animation
#ifdef DO_ANIM
	ClearDisplay();
	delay(500);
	DisplayAnim(anim, anim_durs, sizeof(anim) / 8);
#endif

	// Flash Display
#ifdef DO_FLASH
	for (int i = 0; i < 5; i++) {
		FillDisplay();
		delay(250);
		ClearDisplay();
		delay(250);
	}
#endif

	// Display some icons
#ifdef DO_ICONS
	invertDisplay = false;
	ClearDisplay();
	DisplaySplash(frown_bmp, MAX_BRIGHT);

	delay(1000);
	DisplaySplash(neutral_bmp, MAX_BRIGHT);

	delay(1000);
	DisplaySplash(smile_bmp, MAX_BRIGHT);

	delay(1000);
	DisplaySplash(heart, MAX_BRIGHT);
	invertDisplay = true;

	ClearDisplay();
	DisplaySplash(frown_bmp, MAX_BRIGHT);

	delay(1000);
	DisplaySplash(neutral_bmp, MAX_BRIGHT);

	delay(1000);
	DisplaySplash(smile_bmp, MAX_BRIGHT);

	delay(1000);
	DisplaySplash(heart, MAX_BRIGHT);

	invertDisplay = false;
#endif

	// Light up pixels in sequence and back again
#ifdef DO_SEQUENCE
	delay(1000);
	ClearDisplay();
	for (int c = 0; c < 8; c++) {
		for (int r = 0; r < 8; r++) {
			DrawPixel(c, r, MAX_BRIGHT);
			delay(25);
		}
	}
	for (int c = 7; c >= 0; c--) {
		for (int r = 7; r >= 0; r--) {
			DrawPixel(c, r, MIN_BRIGHT);
			delay(25);
		}
	}

	for (int r = 0; r < 8; r++) {
		for (int c = 0; c < 8; c++) {
			DrawPixel(c, r, MAX_BRIGHT);
			delay(25);
		}
	}
	for (int r = 7; r >= 0; r--) {
		for (int c = 7; c >= 0; c--) {
			DrawPixel(c, r, MIN_BRIGHT);
			delay(25);
		}
	}
#endif
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * mode: c
 * End:
 */
