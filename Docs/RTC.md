# RTC
This module enables the Read Time Clock.

Even when date and time tracking is not required, this module is useful for enabling wakeups from STOP mode - which is a requirement for lower power.

# Usage

## Basic usage:

A `DateTime_t` structure is provided for convenience. The time will be in 24 hour format.

```c
RTC_Init();

// Set current time to 2021/01/01 00:00:00
DateTime_t dt1 = {
    .year = 2021,
    .month = 1,
    .day = 1,
    .hour = 0,
    .minute = 0,
    .second = 0
};
RTC_Write(&dt1);

...

DateTime_t dt2;
RTC_Read(&dt2);
// The datetime can now be read.
```

## Alarms:

The RTC can be used to generate alarms based off the time. The following example will generate an interrupt on the minute. In this example `User_Callback` is a user defined function.

```c
RTC_Init();
RTC_OnAlarm(RTC_Alarm_A, NULL, RTC_Mask_Minute, User_Callback);

while(1)
{
    ...
    // note that the RTC wakeups can be used to exit STOP mode
    CORE_Stop();
}
```

The alarm time can also be set to a specific time. When not provided, the DT argument is assumed to be `00:00:00`.
```c
DateTime_t dt = {
    .minute = 30,
    .second = 21
};
// This will occurr each hour at **:30:21
RTC_OnAlarm(RTC_Alarm_A, &dt, RTC_Mask_Hour, User_Callback);
```

Note that the alarms can be generated each second/hour/minute/day. For more custom periods - the alarm will have to be reinitialised with a new value each wakeup.

## Timer

The RTC may also have an input timer. This can be used for higher frequency wakeups. Check your datasheet for the presence of this device.

```c
RTC_Init();

// Run this callback every 250ms
RTC_OnPeriod(250, User_Callback);

while(1)
{
    ...
    CORE_Stop();
}
```

## Oscillator selection

Its important to note that the LSI (Low speed internal oscillator) is **extremely** inacurate. Check your parts datasheet. It is useful for wakeups, but not for accurate time keeping.

See the [CLK](CLK.md) module for using the LSE (low speed external oscillaor) instead.

# Board

The module is dependant on definitions within `Board.h`
The following template can be used. Commented out definitions are optional.

```C
// RTC configuration
//#define RTC_USE_IRQS
```