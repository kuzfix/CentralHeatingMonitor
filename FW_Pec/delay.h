/*
 * delay.h - delay function remapping
 *
 * Created: 06.06.2012 18:30:07
 *  Author: maticpi
 */ 


#ifndef DUMMY_DELAY_H_
#define DUMMY_DELAY_H_

#ifndef F_CPU
	#include "config.h"
	#ifndef F_CPU
		#define F_CPU 18432000UL
	#endif
#endif

#include <util/delay.h>

#define delay_ms(x) _delay_ms(x)
#define delay_us(x) _delay_us(x)



#endif /* DUMMY_DELAY_H_ */