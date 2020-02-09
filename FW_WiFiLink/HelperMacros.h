#pragma once

#define StringF(x)	String(F(x))
#define printF(x)	print(String(F(x)))
#define printlnF(x)	println(String(F(x)))

// !!! BE CAREFUL WHEN USING STRINGS STORED IN FLASH !!! 
// Wrong type of access causes segmentation fault
// Read more here: https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html
// Summary:
//
// Reusable strings stored in flash:
//  static const char xyz[] PROGMEM = "This is a string stored in flash. Len = %u";
//
//  void setup() {
//		Serial.begin(115200); Serial.println();
//		Serial.println(FPSTR(xyz)); // just prints the string, must convert it to FlashStringHelper first using FPSTR().
//		Serial.printf_P(xyz, strlen_P(xyz)); // use printf with PROGMEM string
//	}
//
// Inline strings for one time usage to be stored in flash:
//	void setup() {
//	    Serial.begin(115200); Serial.println();
//	    Serial.println(F("This is an inline string")); //
//	    Serial.printf_P(PSTR("This is an inline string using printf %s"), "hello");
//	    String xyz = StringF("Stored in flash") + String(10);
//	    Serial.print(xyz);
//	    Serial.print(F("Another string in flash"));
//	}
//	
// or with the use of the above macros:
//	
//	void setup() {
//	    Serial.begin(115200); Serial.println();
//	    Serial.printlnF("This is an inline string"); //
//	    Serial.printf_P(PSTR("This is an inline string using printf %s"), "hello");
//	    String xyz = StringF("Stored in flash") + String(10);
//	    Serial.print(xyz);
//	    Serial.printF("Another string in flash");
//	}
