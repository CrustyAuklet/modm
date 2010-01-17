#include <avr/interrupt.h>

#include <xpcc/hal/peripheral/gpio.hpp>
#include <xpcc/hal/peripheral/software_spi.hpp>
#include <xpcc/hal/time/delay.hpp>
#include <xpcc/hal/peripheral/avr/xmega/spi.hpp>

#include <xpcc/driver/lcd/dog_m16x.hpp>
#include <xpcc/driver/debounce.hpp>

#include <xpcc/io/iostream.hpp>
#include <xpcc/io/iodevice_wrapper.hpp>

#include <xpcc/utils/misc.hpp>

using namespace xpcc;

#if 1
CREATE_OUTPUT_PIN(Led0, H, 0);
CREATE_OUTPUT_PIN(Led1, H, 1);
CREATE_OUTPUT_PIN(Led2, H, 2);
CREATE_OUTPUT_PIN(Led3, H, 3);
CREATE_OUTPUT_PIN(Led4, H, 4);
CREATE_OUTPUT_PIN(Led5, H, 5);
CREATE_OUTPUT_PIN(Led6, H, 6);
CREATE_OUTPUT_PIN(Led7, H, 7);
#else
CREATE_OUTPUT_PIN(Led0, A, 0);
CREATE_OUTPUT_PIN(Led1, A, 1);
CREATE_OUTPUT_PIN(Led2, A, 2);
CREATE_OUTPUT_PIN(Led3, A, 3);
CREATE_OUTPUT_PIN(Led4, A, 4);
CREATE_OUTPUT_PIN(Led5, A, 5);
CREATE_OUTPUT_PIN(Led6, A, 6);
CREATE_OUTPUT_PIN(Led7, A, 7);
#endif
CREATE_INPUT_PIN(Button0, B, 0);
CREATE_INPUT_PIN(Button1, B, 1);
CREATE_INPUT_PIN(Button2, B, 2);
CREATE_INPUT_PIN(Button3, B, 3);

CREATE_INPUT_PIN(EncoderA, C, 0);
CREATE_INPUT_PIN(EncoderB, C, 1);
CREATE_INPUT_PIN(EncoderButton, C, 2);

CREATE_INPUT_PIN(SS, C, 4);

Debounce keys(0x07);
Debounce encoder;

struct DmaPayload{
	char string[24];
	float f;
	uint16_t ii;
	int8_t i;
	bool b;
};

volatile DmaPayload dmaPayload;

CREATE_SPI_MODULE(mySpi, SPIC);

void configure_spi_test(){
	mySpi::initialize(SPI_MODE_0_gc);
	mySpi::enableInterrupt(SPI_INTLVL_MED_gc);
}

void configure_dma_test(){
	DMA.CTRL = DMA_ENABLE_bm | DMA_PRIMODE_RR0123_gc;

	//DMA.STATUS == DMA_CH0BUSY_bm; true if buisy
	//DMA.INTFLAGS == DMA_CH0ERRIF_bm | DMA_CH0TRNIF_bm; finish flags
	

//-------------------------------------
// channel configuration
	
//	DMA.CH0.CTRLB |   DMA_CH_CHBUSY_bm // busy flag
//					| DMA_CH_CHPEND_bm  // chanel pending
//					| DMA_CH_ERRIF_bm // error interrupt flag // not autocleared after interrupt
//					| DMA_CH_TRNIF_bm // Transaction complete flag // not autocleared after interrupt

	DMA.CH0.CTRLB =   DMA_CH_ERRINTLVL_OFF_gc // error interrupt enable
					| DMA_CH_TRNINTLVL_OFF_gc // transaction complete interrupt enable
//					| DMA_CH_TRNINTLVL_MED_gc // transaction complete interrupt enable
					;
					
	DMA.CH0.ADDRCTRL = DMA_CH_SRCRELOAD_TRANSACTION_gc  // source is reloaded with initial value
					| DMA_CH_SRCDIR_FIXED_gc			// mode of changing source
					| DMA_CH_DESTRELOAD_TRANSACTION_gc
					| DMA_CH_DESTDIR_INC_gc
					;
	DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_SPIC_gc; // trigger source select
	
	DMA.CH0.TRFCNT = sizeof(DmaPayload);  // block size(16bit), is automatically decremented after each byte was send and restored finaly
	DMA.CH0.REPCNT = 0;   // times a block transfer is performed, or 0 if unlimited, 
							// is automatically decremented after each block was sent, 
							// if repeat in dma.ch.ctrla is set channel will be automatically disabled 
							// as this register reaches 0
	
//	DMA.CH0.SRCADDR(0,1,2) = // 24bit source address
//	DMA.CH0.DESTADDR(0,1,2) = 0; // 24bit destination address
//  or
//	DMA_CH0_SRCADDR(0,1,2) = // 24bit source address
//	DMA_CH0_DESTADDR(0,1,2) = 0; // 24bit destination address
	
//	DMA.CH0.SRCADDR0 = (register8_t)((uint8_t)(&SPIC.DATA));
//	DMA.CH0.SRCADDR1 = ((&SPIC_DATA)>>(8*1))&(0xff);
//	DMA.CH0.SRCADDR2 = ((&SPIC_DATA)>>(8*2))&(0xff);
	
//	DmaPayload* p = (DmaPayload*)DMA_CH0_DESTADDR0; 
//	DMA_CH0_DESTADDR(0,1,2) = &dmaPayload; // 24bit destination address
	
	DMA_CH0_DESTADDR0 = (((uint16_t)&dmaPayload) >> 0) & 0xff;
	DMA_CH0_DESTADDR1 = (((uint16_t)&dmaPayload) >> 8) & 0xff;
	DMA_CH0_DESTADDR2 = 0;

	DMA.CH0.SRCADDR0 = (((uint16_t)&mySpi::getModuleBase().DATA) >> 0) & 0xff;
	DMA.CH0.SRCADDR1 = (((uint16_t)&mySpi::getModuleBase().DATA) >> 8) & 0xff;
	DMA.CH0.SRCADDR2 = 0;

	
	DMA.CH0.CTRLA = DMA_CH_ENABLE_bm // autocleared on transaction fin
					| DMA_CH_REPEAT_bm   // autocleared at last reppetieion
//					| DMA_CH_TRFREQ_bm   // software tranfair request, autocleared at start of transfair
					| DMA_CH_SINGLE_bm
					| DMA_CH_BURSTLEN_1BYTE_gc
					;
}

void configure_dma_test_ch_1(){
	//-------------------------------------
	// channel configuration
		
	//	DMA.CH1.CTRLB |   DMA_CH_CHBUSY_bm // busy flag
	//					| DMA_CH_CHPEND_bm  // chanel pending
	//					| DMA_CH_ERRIF_bm // error interrupt flag // not autocleared after interrupt
	//					| DMA_CH_TRNIF_bm // Transaction complete flag // not autocleared after interrupt

		DMA.CH1.CTRLB =   DMA_CH_ERRINTLVL_OFF_gc // error interrupt enable
						| DMA_CH_TRNINTLVL_MED_gc // transaction complete interrupt enable
						;
						
		DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_TRANSACTION_gc  // source is reloaded with initial value
						| DMA_CH_SRCDIR_INC_gc			// mode of changing source
						| DMA_CH_DESTRELOAD_NONE_gc
						| DMA_CH_DESTDIR_FIXED_gc
						;
		DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_SPIC_gc; // trigger source select
		
		DMA.CH1.TRFCNT = sizeof(DmaPayload);  // block size(16bit), is automatically decremented after each byte was send and restored finaly
		DMA.CH1.REPCNT = 0;   // times a block transfer is performed, or 0 if unlimited, 
								// is automatically decremented after each block was sent, 
								// if repeat in dma.ch.ctrla is set channel will be automatically disabled 
								// as this register reaches 0
		
		DMA_CH1_SRCADDR0 = (((uint16_t)&dmaPayload) >> 0) & 0xff;
		DMA_CH1_SRCADDR1 = (((uint16_t)&dmaPayload) >> 8) & 0xff;
		DMA_CH1_SRCADDR2 = 0;

		DMA.CH1.DESTADDR0 = (((uint16_t)&mySpi::getModuleBase().DATA) >> 0) & 0xff;
		DMA.CH1.DESTADDR1 = (((uint16_t)&mySpi::getModuleBase().DATA) >> 8) & 0xff;
		DMA.CH1.DESTADDR2 = 0;

		
		DMA.CH1.CTRLA = DMA_CH_ENABLE_bm // autocleared on transaction fin
	//					| DMA_CH_REPEAT_bm   // autocleared at last reppetieion
	//					| DMA_CH_TRFREQ_bm   // software tranfair request, autocleared at start of transfair
						| DMA_CH_SINGLE_bm
						| DMA_CH_BURSTLEN_1BYTE_gc
						;
}

void configureTimer(TC0_t &timer){
	timer.CTRLA |= TC_CLKSEL_DIV2_gc;
	timer.INTCTRLA |= TC_OVFINTLVL_MED_gc;
	timer.INTCTRLB |= TC_CCAINTLVL_MED_gc | TC_CCBINTLVL_MED_gc;
	
	timer.PER = 1000; // period
	timer.CCA = 500; // duty cycle
}

void timerUpdate(TC0_t &timer){
	static bool up = 1;
	int16_t t;
	if (up){
		t = timer.CCA + 40;
	}
	else{
		t = timer.CCA - 40;
	}
	if (t >= 1000 || t <= 0){
		up = !up;
	}
	else{
		timer.CCA = t;
	}
}

int8_t encoderRead(){
	static int8_t table[] = {0,1,-1,0, -1,0,0,1, 1,0,0,-1, 0,-1,1,0};
	static uint8_t state_old = 0;
	static int8_t pos = 0;
	
	uint8_t state = (EncoderA::get()?1:0) | (EncoderB::get()?2:0);
	uint8_t lookup = state_old | (state<<2);
	pos += table[lookup];
	state_old = state;
	
	int8_t step = 0;
	if (pos < 0)
		step = -1;
	else if (pos >= 4)
		step = 1;
	pos &= 0x03;
	
	return step;
	
}

TC0_t &timer = TCD0;

int
main()
{
	Led0::output();
	Led1::output();
	Led2::output();
	Led3::output();
	Led4::output();
	Led5::output();
	Led6::output();
	Led7::output();
	
	Button0::configure(gpio::PULLUP);
	Button1::configure(gpio::PULLUP);
	Button2::configure(gpio::PULLUP);
	Button3::configure(gpio::PULLUP);
	
	EncoderButton::configure(gpio::PULLUP);
	
	SS::configureInputSense(::xpcc::gpio::RISING);
	SS::configureInterrupt0(::xpcc::gpio::INT0LVL_MED);
	
	
	Led6::set();
	Led7::reset();
	delay_ms(100);
	
	configure_spi_test();
	configure_dma_test();
//	configure_dma_test_ch_1();
	
	// enable medium interrupts
	PMIC.CTRL |= PMIC_MEDLVLEX_bm;
	
	configureTimer(timer);

	// enable global interrupts
	sei();
	Led7::set();
	delay_ms(100);
	Led7::reset();
	
	while (1)
	{
//		delay_ms(10);
		
//		timerUpdate(timer);
		
//		keys.update(PORTB.IN);
		
		if (keys.getPress(Debounce::KEY0) || keys.getRepeat(Debounce::KEY0)) {
			Led0::toggle();
		}
		if (keys.getPress(Debounce::KEY1) || keys.getRepeat(Debounce::KEY1)) {

		}
		
		if (keys.getShortPress(Debounce::KEY2)) {

		}
		if (keys.getLongPress(Debounce::KEY2)) {

		}
		
		if (keys.getPress(Debounce::KEY3)) {

		}
		
//		Led6::set(EncoderA::get());
//		Led7::set(EncoderB::get());
		
		// encoder button
		if (encoder.getPress(Debounce::KEY2)) {
//			Led5::toggle();
		}
		
		timer.CCB = dmaPayload.i*10;
		// Led0::set(dmaPayload.b);
		Led6::set(dmaPayload.b);
		
		
//		timer.CCB = 90*10;
	}

}

ISR(SPIC_INT_vect){
//	Led0::toggle();
//	timer.CCB = SPIC.DATA*10;
}

ISR(DMA_CH0_vect){
	Led1::toggle();
	DMA.CH0.CTRLB |= DMA_CH_TRNIF_bm; // clear the flag. necessary if interrupt is enabled.
}

ISR(DMA_CH1_vect){
//	DMA.CH1.CTRLB |= DMA_CH_TRNIF_bm; // clear the flag. maybe necessary if interrupt is enabled.
}

ISR(TCD0_OVF_vect){
//	Led6::reset();
//	Led7::reset();
	
	static uint8_t i = 0;
	if (i++ == 20){
		i=0;
		timerUpdate(timer);
		keys.update(PORTB.IN);
	}
}

ISR(TCD0_CCA_vect){
//	Led6::set();
}

ISR(TCD0_CCB_vect){
//	Led7::set();
}

ISR(PORTC_INT0_vect){
//	static bool errorCondition = 1;
	uint8_t state = DMA_CH0_CTRLB;
	if (state & DMA_CH_CHBUSY_bm){
		Led0::set();
	}
	else{
		Led0::reset();
	}
//	if (errorCondition){
//		DMA_CH0_CTRLA |= DMA_CH_ENABLE_bm;
//		errorCondition = 0;
//	}
//	else{
//		if ()
//	}
//	Led0::toggle();
}
//ISR(_VECTOR(SS::getInterruptVector()){
	
//}
