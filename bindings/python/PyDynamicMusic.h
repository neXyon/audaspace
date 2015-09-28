#pragma once

#include <Python.h>
#include "Audaspace.h"

typedef void Reference_DynamicMusic;

typedef struct {
	PyObject_HEAD
		Reference_DynamicMusic* dynamicMusic;
} DynamicMusic;

extern AUD_API PyObject* DynamicMusic_empty();
extern AUD_API DynamicMusic* checkDynamicMusic(PyObject* dynamisMusic);

bool initializeDynamicMusic();
void addDynamicMusicToModule(PyObject* module);