#include "MyQueue.h"

MyQueue <float> queue(3);

void setup() {

	Serial.begin( 115200 );

	int i;
	for( int i=0; i<4; i++){

		if( i == 3 ){
			Serial.println("RESIZE => 6 ");
			queue.resize(5);
			Serial.println();
		}

		queue.push( i );

		Serial.print( "Ajout : " );
		Serial.print( (float) i );
		Serial.print( " -> passage à ");
		Serial.print( queue.count() );
		Serial.println(" elements :" );

		queue.print();

		for( int j=0; j<queue.count(); j++ ){
			Serial.println( queue.peek(j) );
		}
		Serial.println();
	}

	for( int k=0; k< 5; k++){
		if( !queue.isEmpty() ){
			float buf = queue.pop();

			Serial.print( "Suppression -> passage à " );
			Serial.print( queue.count() );
			Serial.println(" elements :" );
			Serial.print( "Element supprimé : " );
			Serial.println( buf );

			queue.print();

			for( int j=0; j<queue.count(); j++ ){
				Serial.println( queue.peek(j) );
			}
			Serial.println();
		}
	}

	for( int i=0; i<4; i++){

		if( i == 3 ){
			Serial.println("RESIZE => 6 ");
			queue.resize(6);
			Serial.println();
		}

		queue.push( i );

		Serial.print( "Ajout : " );
		Serial.print( (float) i );
		Serial.print( " -> passage à ");
		Serial.print( queue.count() );
		Serial.println(" elements :" );

		queue.print();

		for( int j=0; j<queue.count(); j++ ){
			Serial.println( queue.peek(j) );
		}
		Serial.println();
	}

	Serial.println("fini!");

}

void loop() {

}
