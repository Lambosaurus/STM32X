# TIM
This module enables the timer modules.

This enables a wide variety of functionaly: such as timekeeping, interrupt generation, and PWM.

# Usage

## Read the datasheet:
Note the specific width of each timer. Most timers are 16 bit, but some timers may be 8 or even 32 bit.

Timer channels are used for PWM and IRQ's. Most timers have 2 channels, but some may have 0 or even 4 channels.

## Basic usage:

This module will refer to two terms. These should not be confused:
* **Base frequency:** the frequency of each clock tick. This is the rate at which the timer counts up.
* **Reload frequency:** the frequency of reload events. Reloads are when the timer resets back to 0. This is the the reload value divided by the base frequency. For example, a timer with a 1KHz base frequency, and a reload value of 99, will have a reload frequency of 10Hz. 

It is important to note that the total number of ticks before the counter reloads will be reload + 1; ie, with a reload of `255`, the reload period is `256` ticks in total.

Note that the reload value should not exceed the timers width (ie, `0xFFFF` for a 16 bit timer).

```c
// Set up a 10KHz counter that reloads at 0xFFFF
// This means the timer will reload every 6.5536 seconds.
TIM_Init(TIM_2, 10000, 0xFFFF);
TIM_Start(TIM_2);

...
// Get the current value of the timer.
uint32_t t = TIM_Read(TIM_2);
```

The timer can be started and stopped arbitrarially.

## Interrupts:

An interrupt can be generated when a timer reloads.

In the following example, The user defined `User_Callback` will be called at 10Hz as the timer reloads.

```c
TIM_Init(TIM_2, 10000, 999);
TIM_OnReload(TIM_2, User_Callback);
TIM_Start(TIM_2);

while (1)
{
    CORE_Idle();
}
```

Interrupts can also be generated using the channels.

```c
// Set up a timer with a reload period of 1s in steps of 100
TIM_Init(TIM_2, 100, 99);

// User_Callback1 will occurr 0.1s after the timer starts
TIM_OnPulse(TIM_2, TIM_CH1, User_Callback1);
TIM_SetPulse(TIM_2, TIM_CH1, 9);

// User_Callback2 will occurr 0.2s after the timer starts
TIM_OnPulse(TIM_2, TIM_CH2, User_Callback2);
TIM_SetPulse(TIM_2, TIM_CH2, 19);

TIM_Start(TIM_2);

while (1)
{
    CORE_Idle();
}
```

## PWM:

Timers can also be used to generate PWM.
Timer channels can only be routed to specific GPIO. Check your datasheet.

The GPIO will be high while the timer value is less than the channels pulse value. Note that that means for 100% duty cycle, the pulse value must exceed the reload value.

```c
// Set up a timer with a reload frequency of 10KHz, with an 8 bit resolution.
TIM_Init(TIM_2, 256 * 10000, 255);

// Route channel 1 to PA0, with a duty cycle of 50%
TIM_EnablePwm(TIM_2, TIM_CH1, GPIOA, GPIO_PIN_0, GPIO_AF2_TIM2);
TIM_SetPulse(TIM_2, TIM_CH1, 127);

// Route channel 2 to PA1, with a duty cycle of 100%
TIM_EnablePwm(TIM_2, TIM_CH2, GPIOA, GPIO_PIN_1, GPIO_AF2_TIM2);
TIM_SetPulse(TIM_2, TIM_CH2, 256); 
```

## Mixing Interrupts & PWM:

Note that mixing IRQ's and PWM is perfectly valid, but channels are a shared resource between them. IRQ's and PWM on the same channel will have the same trigger point.

# Board

The module is dependant on definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// TIM config
#define TIM2_ENABLE
//#define TIM_USE_IRQS
```