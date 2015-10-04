#pragma once

#include <Python.h>
#include "Audaspace.h"

typedef void Reference_DynamicMusic;

typedef struct {
	PyObject_HEAD
		Reference_DynamicMusic* dynamicMusic;
} DynamicMusicP;

extern AUD_API PyObject* DynamicMusic_empty();
extern AUD_API DynamicMusicP* checkDynamicMusic(PyObject* dynamicMusic);

bool initializeDynamicMusic();
void addDynamicMusicToModule(PyObject* module);