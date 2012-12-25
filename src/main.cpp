/*
 * Target: AVR328P
 * Crystal: 16.000Mhz
 */
#include <Arduino.h>


typedef struct {
	int 	current;
	int 	last;

	int		max;
	int		min;
	int 	treshold;

	float 	acceleration;

	int level;

	unsigned long level_ch_time;

} Measure_t;

const uint8_t Measure_PIN = A7;

const uint8_t LEDs_BANK_SIZE = 6;
const uint8_t LEDs_PORT[LEDs_BANK_SIZE] = {8, 9, 10, 11, 12, 13};

const uint8_t Relay_PIN = 2;

const uint8_t LedPWM_PIN = 3;

/* -------------------------------------
 *
 */
inline void Measure_Update(Measure_t *m) {

	uint16_t current_value;

	current_value = analogRead(Measure_PIN);

	current_value = (current_value + m->last)/2;

	// calculate level
	if (current_value > m->max ) {
		//if (m->level < 255) m->level += sqrt(256 - m->level/8);
		if (m->level < 255) m->level += 20;
	} else  {
		if (m->level > 1) m->level -= 7;
	}

	// set values
	m->last = m->current;
	m->current = current_value;

	if (m->level >= 255) m->level=255;
	if (m->level <= 0) m->level=0;

	//Serial.println(current_value);
	//Serial.println( m->level);
	//delay(10);
}

/* -------------------------------------
 *
 */
inline void Measure_Setup(Measure_t *m) {

	uint16_t val;
	uint16_t last_val;
	unsigned long last_time = millis();


	m->min = 1023;
	m->max = 0;

	m->level = 0;

	while (millis()-last_time < 3000) {

		val = analogRead(Measure_PIN);

		if ((val > last_val) && (val-last_val>20))
			continue;

		if ((val < last_val) && (last_val-val>20))
			continue;

		if ( val < m->min )
			m->min = val-2;

		if ( val > m->max )
			m->max = val+2;

		//delay(1);

		last_val = val;
	}

	m->min = m->min / 2;
	m->max = m->max / 2;


}

/* -------------------------------------
 *
 */
static unsigned long LastTimeRelay_5level = 0;
static unsigned long LastTimeEnabled = 0;
static unsigned long LedDimmerTime = 0;

static int LedDimmerValue = 0;
static int LedDimmerStep = 2;

static Measure_t measure = {0};


inline void LEDs_Update(uint8_t val) {

	unsigned long current_millis = millis();

	// update level
	int i = 0;

	for (; i < (val / (256/LEDs_BANK_SIZE) ); ++i ) {
		//digitalWrite(LEDs_PORT[i], HIGH);
		if (i == 2) {
			LedDimmerTime = current_millis;
		}
	}


	for (; i < LEDs_BANK_SIZE; ++i ) {
		//digitalWrite(LEDs_PORT[i], LOW);

		/*
		//debug
		if (current_millis - LastTimeEnabled > 6000 )
			continue;
			*/

		if (i == 4) {
			LastTimeRelay_5level = current_millis;
		}
	}


	// Dimmer
	if (current_millis - LedDimmerTime < 1000 ) {
		analogWrite(LedPWM_PIN, val);

	} else {
		LedDimmerValue += LedDimmerStep;

		if (LedDimmerValue > 255) {
			LedDimmerStep  = -2;
			LedDimmerValue = 255;
		}

		if (LedDimmerValue <= 0) {
			LedDimmerStep = 2;
			LedDimmerValue = 1;
		}

		analogWrite(LedPWM_PIN, LedDimmerValue);
	}

	// YEYEYE
	if (current_millis - LastTimeRelay_5level > 120 ) {

		// Blink before
		analogWrite(LedPWM_PIN, 20);
		delay(100);
		analogWrite(LedPWM_PIN, 255);
		delay(100);
		analogWrite(LedPWM_PIN, 20);
		delay(100);
		analogWrite(LedPWM_PIN, 255);
		delay(100);
		analogWrite(LedPWM_PIN, 20);
		delay(100);
		analogWrite(LedPWM_PIN, 255);
		delay(100);
		analogWrite(LedPWM_PIN, 20);
		delay(100);
		analogWrite(LedPWM_PIN, 255);
		delay(100);
		analogWrite(LedPWM_PIN, 20);

		/*
		delay(1500);
		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], HIGH);
		}
		delay(1500);
		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], LOW);
		}*/

		digitalWrite(Relay_PIN, HIGH);
		delay(800);
		digitalWrite(Relay_PIN, LOW);

		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], LOW);
		}
		delay(1500);

		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], HIGH);
		}
		delay(1500);

		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], LOW);
		}
		delay(1500);

		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], HIGH);
		}
		delay(1500);

		for (int z=0; z < LEDs_BANK_SIZE; ++z ) {
			digitalWrite(LEDs_PORT[z], LOW);
		}

		LastTimeRelay_5level = millis();
		LastTimeEnabled = millis();
		measure.level = 0;
	}
}


/* -------------------------------------
 *
 */
inline void LEDs_Check() {

	int i;

	for (i=0; i < LEDs_BANK_SIZE; ++i ) {
		digitalWrite(LEDs_PORT[i], HIGH);
		delay(100);
	}

	for ( i=LEDs_BANK_SIZE; i > 0; --i ) {
		digitalWrite(LEDs_PORT[i], LOW);
		delay(100);
	}

	for (i=255; i > 0; i-=5 ) {
		analogWrite(LedPWM_PIN, i);
		delay(10);
	}

	for (i=0; i < 255; i+=5 ) {
		analogWrite(LedPWM_PIN, i);
		delay(10);
	}

}


/* -------------------------------------
 *
 */
inline void setup() {

	//Serial.begin(9600);
	//Serial.print("sdasdasdasdasd");

	pinMode(Relay_PIN, OUTPUT);
	digitalWrite(Relay_PIN, LOW);

	pinMode(Measure_PIN, INPUT);

	for (int i=0; i < LEDs_BANK_SIZE; ++i ) {
		pinMode(LEDs_PORT[i], OUTPUT);
	}

	pinMode(LedPWM_PIN, OUTPUT);
	analogWrite(LedPWM_PIN, 255);

	Measure_Setup(&measure);
	LEDs_Check();
}

inline void loop() {

	Measure_Update(&measure);
	LEDs_Update(measure.level);

	delay(20);
}

////////// ---------------------------------- ////////
int main(void) {
	init();
	setup();
	for (;;) {
		loop();
		//if (serialEventRun) serialEventRun();
	}
	return 0;
}

