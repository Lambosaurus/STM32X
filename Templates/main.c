
#include "Core.h"




int main(void)
{
	CORE_Init();

	while(1)
	{
		CORE_Idle();
	}
}

