#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>
#include <driverlib/adc.h>
#include <inc/hw_adc.h>
#include <driverlib/pin_map.h>
#include <driverlib/pwm.h>

uint32_t adc_servo;
uint32_t adc_motor;
volatile int servo_angle = 0;

void ADC0ISR(void){
    uint32_t buf[2];
    ADCIntClear(ADC0_BASE, 2);
    ADCSequenceDataGet(ADC0_BASE, 2, buf);

    adc_servo = buf[0];
    servo_angle = ((adc_servo * 90) / 4095) + 45;

    adc_motor = buf[1];

}

void Servo_Motor(void){
    uint32_t pulse_ticks = 0;

    pulse_ticks = 1000 + ((adc_servo * 1000) / 4095);

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, pulse_ticks);

}

void DC_Motor(void){
    uint32_t pulse_ticks = 0;
    uint32_t speed = 0;

    if(adc_motor >= 1950 && adc_motor <= 2150){

        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1, 0);

        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, 0);
    }

    else if(adc_motor < 1950){

        speed = adc_motor - 2150;

        pulse_ticks = (speed * 100) / (4095 - 2150);

        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0);

        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, pulse_ticks);
    }

    else{

        speed = 1950 - adc_motor;

        pulse_ticks = (speed * 100) / 1950;

        GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_1);

        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, pulse_ticks);
    }
}

void configure(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_16);
    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5 | GPIO_PIN_4);
    GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 20000);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 1500);
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);

    GPIOPinConfigure(GPIO_PE4_M0PWM4);
    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_4);
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, 100);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, 1);
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
    PWMOutputState(PWM0_BASE, PWM_OUT_4_BIT, true);

    ADCHardwareOversampleConfigure(ADC0_BASE, 64);
    ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 1);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH11);
    ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH10 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 2);
    ADCIntRegister(ADC0_BASE, 2, ADC0ISR);
    ADCIntDisable(ADC0_BASE, 2);
    ADCIntEnable(ADC0_BASE, 2);
    ADCIntClear(ADC0_BASE, 2);
    ADCProcessorTrigger(ADC0_BASE, 2);
}

int main(void)
{
	configure();
	while(1){
	    ADCProcessorTrigger(ADC0_BASE, 2);
	    Servo_Motor();
	    DC_Motor();
	}
}
