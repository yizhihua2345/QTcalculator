# QTcalculator.pro
# 带键盘事件的计算器 —— Qt Widgets 工程文件（Qt 6.9.2，qmake）
QT += core gui widgets

CONFIG += c++17

TARGET = QTcalculator
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
