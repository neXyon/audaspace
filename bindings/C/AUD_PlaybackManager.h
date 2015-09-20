#pragma once

#include "AUD_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern AUD_API AUD_PlaybackManager* AUD_PlaybackManager_create(AUD_Device* device);
extern AUD_API void AUD_PlaybackManager_free(AUD_PlaybackManager* manager);
extern AUD_API void AUD_PlaybackManager_play(AUD_PlaybackManager* manager, AUD_Sound* sound, const char* catName);
extern AUD_API int AUD_PlaybackManager_resume(AUD_PlaybackManager* manager, const char* catName);
extern AUD_API int AUD_PlaybackManager_pause(AUD_PlaybackManager* manager, const char* catName);
extern AUD_API float AUD_PlaybackManager_getVolume(AUD_PlaybackManager* manager, const char* catName);
extern AUD_API int AUD_PlaybackManager_setVolume(AUD_PlaybackManager* manager, float volume, const char* catName);
extern AUD_API int AUD_PlaybackManager_stop(AUD_PlaybackManager* manager, const char* catName);
extern AUD_API void AUD_PlaybackManager_clean(AUD_PlaybackManager* manager);

#ifdef __cplusplus
}
#endif