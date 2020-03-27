
/* Arduino SAMD21 Timer
 * 
 * Based on https://gist.github.com/nonsintetic/ad13e70f164801325f5f552f84306d6f
 * 
 * Include this header file in main.cpp
 * Add the TC5_Handler() callback function to the code (DO NOT RENAME - NAME IS SPECIFIED BY ARDUINO)
 * Call tcConfigure(FREQ) in the setup area of the code
 * Call tcStartCounter() to begin
 * 
 */

/*
void TC5_Handler(void) {
  
  // TC5 INTERRUPT CODE HERE

  TC5->COUNT16.INTFLAG.bit.MC0 = 1; //Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}
*/

// Configures the TC to generate output events at the sample frequency.
// Configures the TC in Frequency Generation mode, with an event output once
//   each time the sample frequency period expires.
void tcConfigure(float sampleRate) {

  // Enable GCLK for TCC2 and TC5 (timer counter input clock)
  GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
  while (GCLK->STATUS.bit.SYNCBUSY);

  tcReset(); //reset TC5

  // Set Timer counter Mode to 16 bits
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  // Set TC5 mode as match frequency
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  //set prescaler and enable TC5
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_ENABLE; //you can use different prescaler divisons here like TC_CTRLA_PRESCALER_DIV1 to get different ranges of frequencies
  //set TC5 timer counter based off of the system clock and the user defined sample rate or waveform
  TC5->COUNT16.CC[0].reg = (uint16_t) (SystemCoreClock / (sampleRate*1000.0f) - 1);
  while (tcIsSyncing());
 
  // Configure interrupt request
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);

  // Enable the TC5 interrupt request
  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  while (tcIsSyncing()); //wait until TC5 is done syncing 
} 

// Check if TC5 is done syncing
// returns true when it is done syncing
bool tcIsSyncing(void) {
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

// Enable TC5 and waits for it to be ready
void tcStartCounter(void) {
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; //set the CTRLA register
  while (tcIsSyncing()); //wait until snyc'd
}

// Reset TC5 
void tcReset(void) {
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (tcIsSyncing());
  while (TC5->COUNT16.CTRLA.bit.SWRST);
}

// Disable TC5
void tcDisable(void) {
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tcIsSyncing());
}