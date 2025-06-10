# Beaglebone Home Security System

Course: EC535 – Embedded Systems  
Contributors: Abigail Skerker, Cristian Palencia  
Platform: Beaglebone Black  
Kernel Module Language: C  
Demo Video: [YouTube](https://youtu.be/EEVpLaNmyOU)  
Source Code: [GitHub Repository](https://github.com/cpalencica/SecuritySystem)

---

## Project Overview

This project implements a simple home security system on a Beaglebone Black using a custom Linux kernel module. The system detects motion using a PIR sensor and triggers a buzzer alarm if motion is detected while the system is active. A pushbutton toggles the active mode. The buzzer is controlled via software-emulated PWM using kernel threads.

---

## Components Used

- Beaglebone Black
- HC-SR501 PIR Motion Sensor
- Pushbutton
- Buzzer
- Linux Kernel Module (written in C)

---

## System Architecture

> Figure 1: High-Level System Diagram
>
> ![System Diagram Placeholder](./figures/figure1.png)

- The motion sensor triggers an interrupt when motion is detected.
- A pushbutton toggles system mode (armed/disarmed).
- A kernel timer and kernel thread handle buzzer behavior and PWM output.

---

## Kernel Module Overview
- Initialization: GPIOs are requested and configured; interrupts are registered; character device is initialized.
- Interrupts:
  - Motion sensor ISR triggers a 5-second buzzer alert if system is active.
  - Pushbutton ISR toggles the active/inactive system mode.
- PWM Generation: A kernel thread toggles the buzzer pin using `udelay()` to simulate PWM (~2 kHz).
- Character Device: Provides read access for user-space applications to see system status (active timers, system mode).

---

## File Descriptions

| File | Description |
|------|-------------|
| `security.c` | Main kernel module containing device logic |
| `Makefile` | Build script for kernel module |

---
## Hardware Setup

> Figure 2: Circuit Diagram  
> ![Circuit Diagram Placeholder](./figures/figure2.png)

This project uses the following hardware components connected as described below:

- **Beaglebone Black**: Acts as the main controller running the Linux kernel module.
- **PIR Motion Sensor (HC-SR501)**: Connected to a GPIO pin configured to trigger interrupts when motion is detected. It requires 5V power and ground.
- **Pushbutton**: Connected to a GPIO input pin with an appropriate pull-up or pull-down resistor to detect user toggling between "armed" and "disarmed" system modes.
- **Buzzer**: Connected to a GPIO output pin. Controlled via software PWM generated in the kernel module to produce an audible alarm.

### Wiring Summary

| Component       | Beaglebone Pin          | Notes                            |
|-----------------|-------------------------|---------------------------------|
| PIR Sensor VCC  | 5V                      | Power supply                    |
| PIR Sensor GND  | GND                     | Ground                         |
| PIR Sensor OUT  | GPIO                    | Interrupt input from sensor    |
| Pushbutton      | GPIO                    | Input with pull-up/pull-down   |
| Buzzer          | GPIO                    | PWM output to buzzer           |

---

## Major Code Components

### 1. Interrupt Handling
```c
sensor_irq = gpio_to_irq(sensor_pin);
request_irq(sensor_irq, sensor_irq_handler, IRQF_TRIGGER_RISING, ...);
```

- Motion sensor generates a rising-edge interrupt.
- Calls sensor_irq_handler() which activates the buzzer.

### 2. Pushbutton Toggle
```c
response0 = gpio_request(button_pin, "sysfs");
request_irq(gpio_to_irq(button_pin), button_irq_handler, ...);
```

- Toggled between "armed" and "disarmed" mode.

### 3. Kernel Timer and kthread
```c
etx_timer = kmalloc(...);
timer_setup(etx_timer, buzzer_timer_callback, 0);
```

- Starts a 5-second countdown after motion is detected.
- Buzzer is stopped when timer expires.

### 4. Software PWM Thread

``` c
buzzer_thread = kthread_run(pwm_thread_function, NULL, ...);
``` 

- Controls duty cycle by toggling the buzzer pin in a loop using udelay().

--- 
## PWM Thread

The PWM (Pulse Width Modulation) thread is a key part of the kernel module responsible for generating a PWM signal on a GPIO pin to drive the buzzer. Since the buzzer requires a modulated signal to produce an audible tone, the kernel module creates a dedicated kernel thread that toggles the buzzer GPIO pin on and off at a specified frequency and duty cycle.

### Overview

- The PWM thread runs continuously while the alarm is active.
- It toggles the buzzer GPIO pin between HIGH and LOW states with precise timing to simulate a square wave.
- The thread uses `msleep()` or `usleep_range()` calls to control the HIGH and LOW durations.
- The frequency and duty cycle parameters control how fast the pin toggles and how long it stays HIGH relative to LOW.

### Thread Workflow

1. **Initialization**: When the alarm is triggered (e.g., motion detected and system armed), the kernel module starts the PWM thread.
2. **Looping**: Inside the thread function, an infinite loop toggles the GPIO pin HIGH and LOW with timing intervals computed from the desired PWM frequency and duty cycle.
3. **Timing Control**:  
   - The period `T = 1 / frequency` (e.g., 1/1000 Hz = 1 ms period for 1 kHz).  
   - HIGH time = `T * duty_cycle` (e.g., 50% duty cycle means HIGH for 0.5 ms).  
   - LOW time = `T - HIGH_time`.
4. **GPIO Control**:  
   - Set GPIO HIGH  
   - Sleep for HIGH time  
   - Set GPIO LOW  
   - Sleep for LOW time
5. **Termination**: The thread checks a `stop` flag to terminate cleanly when the alarm stops or the module unloads.

### Code Snippet (Pseudo-C):

```c
#define PWM_FREQ_HZ 1000
#define PWM_DUTY_CYCLE 50 // Percent

static int pwm_thread_fn(void *data) {
    unsigned int period_us = 1000000 / PWM_FREQ_HZ;
    unsigned int high_time_us = (period_us * PWM_DUTY_CYCLE) / 100;
    unsigned int low_time_us = period_us - high_time_us;

    while (!kthread_should_stop()) {
        gpio_set_value(buzzer_gpio_pin, 1);  // Turn buzzer ON
        usleep_range(high_time_us, high_time_us + 10);

        gpio_set_value(buzzer_gpio_pin, 0);  // Turn buzzer OFF
        usleep_range(low_time_us, low_time_us + 10);
    }

    gpio_set_value(buzzer_gpio_pin, 0);  // Ensure buzzer is OFF before exiting
    return 0;
}
```

---
## Build & Run Instructions
```bash
# Compile
make

# Load module (as root)
sudo insmod home_security.ko

# Check kernel logs
dmesg | tail -n 20

# Read system status
cat /dev/home_security

# Unload
sudo rmmod home_security
```

---
## Figures
- Figure 1: High-level system block diagram
- Figure 2: Circuit Diagram
- Figure 3: Real-life Circuit

---

## Demo Video
Watch the system in action: https://youtu.be/EEVpLaNmyOU