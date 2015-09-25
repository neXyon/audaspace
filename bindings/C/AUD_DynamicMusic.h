#pragma once

#include "AUD_Types.h"
#include "AUD_Handle.h"

#ifdef __cplusplus
extern "C" {
#endif

extern AUD_API AUD_DynamicMusic* AUD_DynamicMusic_create(AUD_Device* device);
extern AUD_API void AUD_DynamicMusic_free(AUD_DynamicMusic* player);
extern AUD_API int AUD_DynamicMusic_addScene(AUD_DynamicMusic* player, AUD_Sound* scene);
extern AUD_API int AUD_DynamicMusic_setSecene(AUD_DynamicMusic* player, int scene);
extern AUD_API int AUD_DynamicMusic_getScene(AUD_DynamicMusic* player);
extern AUD_API void AUD_DynamicMusic_addTransition(AUD_DynamicMusic* player, int ini, int end, AUD_Sound* transition);
extern AUD_API void AUD_DynamicMusic_setFadeTime(AUD_DynamicMusic* player, float seconds);
extern AUD_API float AUD_DynamicMusic_getFadeTime(AUD_DynamicMusic* player);
extern AUD_API int AUD_DynamicMusic_resume(AUD_DynamicMusic* player);
extern AUD_API int AUD_DynamicMusic_pause(AUD_DynamicMusic* player);
extern AUD_API int AUD_DynamicMusic_seek(AUD_DynamicMusic* player, float position);
extern AUD_API float AUD_DynamicMusic_getPosition(AUD_DynamicMusic* player);
extern AUD_API float AUD_DynamicMusic_getVolume(AUD_DynamicMusic* player);
extern AUD_API int AUD_DynamicMusic_setVolume(AUD_DynamicMusic* player, float volume);
extern AUD_API AUD_Status AUD_DynamicMusic_getStatus(AUD_DynamicMusic* player);
extern AUD_API int AUD_DynamicMusic_stop(AUD_DynamicMusic* player);

#ifdef __cplusplus
}
#endif