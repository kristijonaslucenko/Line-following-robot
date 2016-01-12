/*
* adc.h
*
* Created: 2/19/2015 6:40:29 PM
*  Author: Adam Orosz, s134615
*/


#ifndef ADC_H_
#define ADC_H_

#define VSTEP 250

#define get_adc(u16_union_value){u16_union_value.b[0] = ADCL;u16_union_value.b[1] = ADCH;}


extern volatile	uint16_t result0;
extern volatile	uint16_t result1;
extern volatile bool new_data_available;
extern volatile bool new_data_available_to_transmit;
extern volatile bool conversionIsInProgress;
extern volatile bool first_channel;

void enable_adc();
void disable_adc();
void handleMeasurement();
void measureWithBothKindOfSensors();
void measureWithItr8307sOnly();
void measure();
void adc_measurement_init();
void send_adc_value_to_pc();



#endif /* ADC_H_ */