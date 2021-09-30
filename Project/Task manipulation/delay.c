#include "delay.h"
/* Include library driver from TivaWare™ Peripheral Driver Library. */
#include "driverlib/sysctl.h"

/*-------------------DelayMS-----------------------/
* Inputs: ui32MStime - time in miliseconds
* Outputs: none.
*/
void DelayMS(uint32_t ui32MStime){
	/*
	*	1 clock cycle = 1 / SysCtlClockGet() second
	*	1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
	*	1 second = SysCtlClockGet() / 3
	*	0.001 second = 1ms = SysCtlClockGet() / 3 / 1000
	*/
	SysCtlDelay(ui32MStime * (SysCtlClockGet() / 3 / 1000));
}

/*-------------------DelayUS-----------------------/
* Inputs: ui32UStime - time in microseconds
* Outputs: none.
*/
void DelayUS(uint32_t ui32UStime){
	/*
	*	1 clock cycle = 1 / SysCtlClockGet() second
	*	1 SysCtlDelay = 3 clock cycle = 3 / SysCtlClockGet() second
	*	1 second = SysCtlClockGet() / 3
	*	0.000001 second = 1ms = SysCtlClockGet() / 3 / 1000000
	*/
	SysCtlDelay(ui32UStime * (SysCtlClockGet() / 3 / 1000000));
}
