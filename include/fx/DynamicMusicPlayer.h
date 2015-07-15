#pragma once

/**
* @file DynamicMusicPlayer.h
* @ingroup fx
* The DynamicMusicPlayer class.
*/

#include "devices/IHandle.h"
#include "devices/IDevice.h"
#include "ISound.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN

/**
* This class allows to play music depending on a current "scene", scene changes are managed by the class.
* The default scene is silent and has id 0.
*/
class AUD_API DynamicMusicPlayer
{
private:
	/**
	* Matrix of pointers which will store the sounds of the scenes and the transitions between them.
	*/
	std::vector<std::vector<std::shared_ptr<ISound>>*> m_scenes;

	/**
	* Id of the current scene.
	*/
	int m_id;

	/**
	* Length of the crossfade transition in seconds, used when no custom transition has been set.
	*/
	float m_fadeTime;

	/**
	* Handle to the playback of the current scene.
	*/
	std::shared_ptr<IHandle> m_currentHandle;

	/**
	* Device used for playback.
	*/
	std::shared_ptr<IDevice> m_device;

	struct PlayData{
		std::shared_ptr<IDevice> device;
		std::shared_ptr<ISound> sound;
		std::shared_ptr<IHandle>* handle;
	} m_pData;

	// delete copy constructor and operator=
	DynamicMusicPlayer(const DynamicMusicPlayer&) = delete;
	DynamicMusicPlayer& operator=(const DynamicMusicPlayer&) = delete;

public:
	/**
	* Creates a new dynamic music manager with the default silent scene (id: 0).
	* \param device The device that will be used to play sounds.
	*/
	DynamicMusicPlayer(std::shared_ptr<IDevice> device);

	virtual ~DynamicMusicPlayer();

	/**
	* Adds a new scene to the manager.
	* \param sound The sound that will play when the scene is selected with the changeScene().
	* \return The identifier of the new scene.
	*/
	int addScene(std::shared_ptr<ISound> sound);

	/**
	* Changes to another scene.
	* \param id The id of the scene which should start playing the changeScene method.
	* \exception Exception An exception will be thrown if the selected scene doesn't exist.
	*/
	void changeScene(int id);

	/**
	* Changes to another scene.
	* \param id The id of the scene which should start playing the changeScene method.
	* \exception Exception An exception will be thrown if the selected scene doesn't exist.
	*/
	void addTransition(int init, int end, std::shared_ptr<ISound> sound);

	/**
	* Sets the length of the crossfade transition (default 1 second).
	* \param seconds The time in seconds.
	*/
	void setFadeTime(float seconds);

	/**
	* Gets the length of the crossfade transition (default 1 second).
	* \return The length of the cressfade transition in seconds.
	*/
	float getFadeTime();

private:
	void transition(int init, int end);
};

AUD_NAMESPACE_END