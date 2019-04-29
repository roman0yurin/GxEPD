cmake_minimum_required(VERSION 3.7)

PROJECT(GxEPD C CXX ASM)

#библиотека для работы со шрифтами.
addSubmodule(../U8g2_fonts)

configureModule()
