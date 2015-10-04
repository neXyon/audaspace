#pragma once

#include <Python.h>
#include "Audaspace.h"

typedef void Reference_PlaybackManager;

typedef struct {
	PyObject_HEAD
	Reference_PlaybackManager* playbackManager;
} PlaybackManagerP;

extern AUD_API PyObject* PlaybackManager_empty();
extern AUD_API PlaybackManagerP* checkPlaybackManager(PyObject* playbackManager);

bool initializePlaybackManager();
void addPlaybackManagerToModule(PyObject* module);