Firmware version------LBS_1.0
Hardware version------Supports VL_1.4/VL_1.3
Updated date: 10-01-2025
Updated by: SAI GOUD & GOPI
1.Load balancing System 
2.Phase Assign
3.Phase Change
4.Load Change
 






Firmware version------Slave_Type-2_V1.1.4
Hardware version------Supports VL_1.4/VL_1.3
Updated date: 12-12-2023
Updated by: Shiva Poola & Abhigna

1. Driving LED's 
2. Relay toggle removed when the PWM starts.(Master initiate the relay driving only for stop, slave itself take caring the relay driving based on CP status for start)
3. Emergency reading handle by slave.
4. Changed software serial to hardware serial with same pins(default UART2 serial - Serial2)
5. Partition scheme changed to minimal spiffs 1.9MB
6. Commented STATE E Relay OFF
7. Stopping Relay before PWM stop.




=======================================
Device Configurations
Debug&Programming UART0(115200)
LED(GPIO)
Realy(GPIO)
Emergency(GPIO)
Hardware Serial(Connected to Master)
Control Pilot IN(ADC)
Control Pilot OUT(PWM)



