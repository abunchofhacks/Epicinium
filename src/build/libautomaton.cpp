// cl /c /EHcs /Z7 /O2 /std:c++14 /Fo.\bin\epicinium-automaton.obj /I.\ /I.\src\common\ /I.\src\logic\ /I.\src\ai /DWIN32_LEAN_AND_MEAN /DNOMINMAX /DPLATFORMWINDOWS64 .\src\build\libautomaton.cpp
// lib .\bin\epicinium-automaton.obj

#define INTL_ENABLED false
#define NEURALNEWT_20x13_ENABLED true

#include "../common/.cu-common.cpp"
#include "../common/validation.cpp"
#include "../common/version.cpp"
#include "../logic/.cu-automaton.cpp"
#include "../logic/.cu-logic.cpp"
#include "../logic/bible.cpp"
#include "../ai/.cu-ai.cpp"
#include "../ai/aichargingcheetah.cpp"
