cmake_minimum_required(VERSION 3.7)

PROJECT(GxEPD C CXX ASM)

#EPAPER_MODEL - название используемого в проекте экранчика eInk

file(GLOB ADD_SRC
		src/*.cpp src/*.c
#		src/GxIO/*.cpp эти классы обеспечивают низкоуровневую связь с MCU, дублируют возможности которые есть в платформе C60
#		${CMAKE_CURRENT_SOURCE_DIR}/src/GxIO/GxIO_SPI/*.cpp
		src/${EPAPER_MODEL}/*.cpp
)

additionalImport(${CMAKE_CURRENT_SOURCE_DIR}/src/${EPAPER_MODEL})
additionalSources("${ADD_SRC}")


#библиотека для работы со шрифтами.
addSubmodule(../../../kernel/rtos)
addSubmodule(../AdafruitGFX)

configureModuleSpecSrc()
