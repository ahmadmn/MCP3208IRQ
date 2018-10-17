#include <SPI.h>
#include "RevBits.h"
#define SAMPLING_PERIOD_US  50  //tested down to 20us
/***********************************************************************************************/
static inline void setDataBits(uint16_t bits) {
  const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
  bits--;
  SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}
/***********************************************************************************************/
void spiBegin(void) {
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(LSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16); //1mhz
  SPI.setHwCs(1);
  setDataBits(19);
}
/***********************************************************************************************/
#define ICACHE_RAM_ATTR     __attribute__((section(".iram.text")))
static inline ICACHE_RAM_ATTR uint16_t transfer_spi_MC3208(byte ch) {
  union {
    uint16_t val;
    struct {
      uint8_t lsb;
      uint8_t msb;
    };
  } out;
  uint32_t spi_val = SPI1W0;
  SPI1W0 = Reverse3Bit[ch];
  SPI1CMD |= SPIBUSY;
  out.val = (spi_val >> 3) & 0xFFFF;
  uint8_t tmp = ReverseByte[out.msb];
  out.msb = ReverseByte[out.lsb] & 0x0F;
  out.lsb = tmp;
  return out.val;
}
/***********************************************************************************************/
void ICACHE_RAM_ATTR sample_isr(void) {
  uint16_t adc_sample = transfer_spi_MC3208(next_ch);
}
/***********************************************************************************************/
void setup(void) {
  spiBegin();
  timer1_isr_init();
  timer1_attachInterrupt(sample_isr);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
  timer1_write((clockCyclesPerMicrosecond() * SAMPLING_PERIOD_US) / 16 );
}

#pragma GCC push_options
#pragma GCC optimize("O3")
/***********************************************************************************************/
void loop() {
}
