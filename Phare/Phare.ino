#include <Adafruit_NeoPixel.h>

#define PIXELPIN	6
#define NUMPIXELS	12
Adafruit_NeoPixel pixels(NUMPIXELS, PIXELPIN, NEO_GRB + NEO_KHZ800);

unsigned long time;
unsigned long stepTime = 150;

int startPixel = 0;

void setup(){

	pixels.begin();
}

void loop(){

	if( millis() > time + stepTime ){
		time += stepTime;

		for( int i=0; i<NUMPIXELS; i++ ){
			if( i == (startPixel % NUMPIXELS) || i == ((startPixel+1) % NUMPIXELS) || i == ((startPixel+2) % NUMPIXELS) ){
				pixels.setPixelColor( i, pixels.Color(150, 150, 150) );
			} else {
				pixels.setPixelColor( i, pixels.Color(0, 0, 0) );
			}
		}

		pixels.show();

		startPixel++;
		if( startPixel >= NUMPIXELS ) startPixel = 0;

	}

}
