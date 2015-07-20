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
#include <thread>
#include <atomic> 
#include <condition_variable>
#include <mutex>

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
	std::vector<std::vector<std::shared_ptr<ISound>>> m_scenes;

	/**
	* Id of the current scene.
	*/
	std::atomic_int m_id;

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

	std::atomic_bool m_transitioning;
	std::atomic_int m_soundTarget;

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
	* \return
	*        - true if the change has been scheduled succesfully.
	*        - false if there already is a transition in course, the scene selected doesnt exist, or the scene scene selected is already playing.
	*/
	bool changeScene(int id);

	/**
	* Retrieves the scene currently selected.
	* \return The identifier of the current scene.
	*/
	int getScene();

	/**
	* Adds a new transition between scenes
	* \param init The id of the initial scene that will allow the transition to play.
	* \param end The id if the target scene for the transition.
	* \param sound The sound that will play when the scene changes from init to end.
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

	/**
	* Resumes a paused sound.
	* \return
	*        - true if the sound has been resumed.
	*        - false if the sound isn't paused or the handle is invalid.
	*/
	bool resume();

	/**
	* Pauses the current played back sound.
	* \return
	*        - true if the sound has been paused.
	*        - false if the sound isn't playing back or the handle is invalid.
	*/
	bool pause();

	/**
	* Seeks in the current played back sound.
	* \param position The new position from where to play back, in seconds.
	* \return
	*        - true if the handle is valid.
	*        - false if the handle is invalid.
	* \warning Whether the seek works or not depends on the sound source.
	*/
	bool seek(float position); 

	/**
	* Retrieves the current playback position of a sound.
	* \return The playback position in seconds, or 0.0 if the handle is
	*         invalid.
	*/
	float getPosition();

	/**
	* Retrieves the pitch of the playing sound.
	* \return The pitch.
	*/
	float getPitch(); 

	/**
	* Sets the pitch of the playing sound.
	* \param pitch The pitch.
	* \return
	*        - true if the handle is valid.
	*        - false if the handle is invalid.
	*/
	bool setPitch(float pitch);

	/**
	* Retrieves the volume of the playing sound.
	* \return The volume. 
	*/
	float getVolume();

	/**
	* Sets the volume of the playing sound.
	* \param volume The volume.
	* \return
	*        - true if the handle is valid.
	*        - false if the handle is invalid.
	*/
	bool setVolume(float volume);

	/**
	* Returns the status of the current played back sound.
	* \return
	*        - STATUS_INVALID if the sound has stopped or the handle is
	*.         invalid
	*        - STATUS_PLAYING if the sound is currently played back.
	*        - STATUS_PAUSED if the sound is currently paused.
	*        - STATUS_STOPPED if the sound finished playing and is still
	*          kept in the device.
	* \see Status
	*/
	Status getStatus();

	/**
	* Stops any played back or paused sound and sets the dynamic music player to default silent state (scene 0)
	* \return
	*        - true if the sound has been stopped.
	*        - false if the handle is invalid.
	*/
	bool stop();

	private:
		static void transitionCallback(void* player);
		static void sceneCallback(void* player);
		static void fadeCallback(void* player);
};

AUD_NAMESPACE_END