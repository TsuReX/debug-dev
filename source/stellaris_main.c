/**
 * @file	src/main.c
 *
 * @brief	Объявление точки вхождения в C-код.
 *
 * @author	Vasily Yurchenko <vasily.v.yurchenko@yandex.ru>
 */


#include <stdint.h>
#include <stdbool.h>

#include "context.h"
#include "config.h"
#include "init.h"
#include "drivers.h"
#include "time.h"

int32_t main()
{
	soc_init();

	board_init();

	while(1) {

		GPIOF->DATA = 2;
		mdelay(50);
		GPIOF->DATA = 4;
		mdelay(50);
		GPIOF->DATA = 8;
		mdelay(50);
	}

	return 0;
}

