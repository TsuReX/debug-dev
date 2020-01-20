Система сборки:

Система сборки основана на cmake и состоит из CMakeLists.txt верхнего уровня, который расположен в корневой директории проекта и опциональных .cmake скриптов, которые располагаются в поддиректории boards и служат для упрощения процесса конфигурирования, они определяют дополнительные цели (flash - для выполнения прошивки), определяют переменные, которые влияют на сам процесс конфигуриования и сборки.

Этапы сборки проекта:

1. Создание директории, в которой будет происходить сборка
	mkdir build

2. Добавление в переменную окружения PATH пути к кросс-компилятору:
	примерный вариант 1
	export PATH=$PATH:"/opt/gcc-arm-none-eabi/bin"
	примерный вариант 2
	export PATH=$PATH:"/home/yurchenko/soft/gcc-arm-none-eabi/bin"

3. Переход в сборочную директорию и создание в ней окружения сборки:
	cd build
	clear ;\
	rm -rf ../build/* ;\
	#вариант конфигурирования 2
	cmake .. -DCMAKE_C_COMPILER=arm-none-eabi-gcc -DDRIVER_PATH=../STM32Cube_FW_F3_V1.11.0/Drivers -DBOARD_TYPE=nucleo_board_f303k8t6
		или
	вариант конфигурирования 1
	cmake .. -DCMAKE_C_COMPILER=arm-none-eabi-gcc -DDRIVER_PATH=../STM32Cube_FW_F3_V1.11.0/Drivers -DMCU_MAJOR_MODEL=STM32F3xx -DMCU_MINOR_MODEL=STM32F303x8 -DUSE_FULL_LL_DRIVER=y
	
	Возможны два варианта конфигурирования сборочного окружения: ручно(первый) и автоматический(второй).
	В случае использования ручного конфигурирования необходимо явно прописывать все параметры.
	В случае использования автоматического конфигурирования используется вспомогательный скрипт из поддиректории boards, который определяет все необходимые параметры, а так же определяет цель flash.
	НО! При использовании автоматического конфигурирования вожможно ручное изменения каких либо параметров.
	
	Переменные, используемые для конфигурирования:
	CMAKE_C_COMPILER - путь и имя компилятора или просто имя, если путь к нему прописан в PATH;
	DRIVER_PATH - путь к драйверам в исходных кодах STM32Cube;
	MCU_MAJOR_MODEL - используемое семейство SoC (STM32F3xx, STM32F4xx);
	MCU_MINOR_MODEL - используемая модель SoC (STM32F303x8, STM32F103xB);
	USE_FULL_LL_DRIVER - использовать библиотеки драйверов полностью (доступны функции и макросы - компилируются заголовочные файлы и файлы с исходниками) или сокращенно (доступны только макросы - компилируются только заголовочники);
	BOARD_TYPE - используемая платформа (nucleo_board_f303k8t6, china_board_f103c8t6) - имя .cmake скрипта из поддиректории boards, который используется для автоматического конфигурирования.
	TODO: Ввести параметр FLASHER_TYPE, который будет поределять, какой .cmake скрипт использовать для определения цели flash для прошивки, и, какой .cfg скрипт программатора использовать.
	
	ВАЖНО! Использование идеологии вспомогательных .cmake скриптов не должно вводить никаких ограничений на работу основного CMakeLists.txt - в основном скрипте не должно быть зависимостей от вспомогательных скриптов, они служат лишь для упрощения кофигурирования - автоматического определения переменных.
	
4. Выполнение сборки
	cmake --build . --clean-first

5. Выполнение прошивки
	cmake --build . --target flash