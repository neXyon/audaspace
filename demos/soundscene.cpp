/*******************************************************************************
* Copyright 2009-2015 Juan Francisco Crespo Galán
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
******************************************************************************/

#include "fx/ConvolverSound.h"
#include "fx/ImpulseResponse.h"
#include "util/ThreadPool.h"
#include "devices/DeviceManager.h"
#include "devices/IDevice.h"
#include "devices/IDeviceFactory.h"
#include "devices/IHandle.h"
#include "plugin/PluginManager.h"
#include "respec/ChannelMapper.h"
#include "file/File.h"
#include "fx/DynamicMusic.h"
#include "fx/SoundList.h"
#include "fx/MutableSound.h"
#include "fx/HRTFLoader.h"
#include "fx/BinauralSound.h"
#include "fx/PlaybackManager.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <cmath>

#include <Windows.h>

#define PI 3.14159265

using namespace aud;

void loadSounds(const std::string& path, const std::string& ext, std::vector<std::shared_ptr<ISound>>& outList);
void fadeInOut(std::shared_ptr<IHandle> handle, float limit, int duration);
void moveSourceStraight(std::shared_ptr<Source> source, float targetAzimuth, float targetElevation, float lineAngle, float minDistance, int duration);
void moveSourceCircle(std::shared_ptr<Source> source, float targetAzimuth, float targetElevation, int duration);
void moveSourceAway(std::shared_ptr<Source> source, float targetDistance, int duration);
std::shared_ptr<IHandle> playSound(std::shared_ptr<PlaybackManager> manager, std::shared_ptr<ISound> sound, float volume = 1.0, int loopCount = 0, float pitch = 1.0, unsigned int category = 0);

int main(int argc, char* argv[])
{
	PluginManager::loadPlugins("");

	auto factory = DeviceManager::getDeviceFactory("SDL");

	auto device = factory->openDevice();
	auto manager = std::make_shared<PlaybackManager>(device);

	auto fftPlan(std::make_shared<FFTPlan>(2048, true));
	auto threadPool(std::make_shared<ThreadPool>(std::thread::hardware_concurrency()));
	auto hrtfs = HRTFLoader::loadLeftHRTFs(fftPlan, ".wav", "hrtfs");
	auto rev_ir(std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(std::make_shared<File>("effects/ir/living_room_1.wav")), fftPlan));
	auto i_ir(std::make_shared<ImpulseResponse>(std::make_shared<StreamBuffer>(std::make_shared<File>("effects/ir/Opti-inverse.wav")), fftPlan));
	DeviceSpecs specs;
	specs.channels = CHANNELS_MONO;
	specs.rate = hrtfs->getSpecs().rate;
	
	auto sourceDoor = std::make_shared<Source>(150, 0, 0.70);
	auto sourcePeople = std::make_shared<Source>(150, 0, 0.95);
	auto sourcePerson = std::make_shared<Source>(150, -40, 0.85);
	auto sourceHands = std::make_shared<Source>(150, 0, 0.85);
	auto sourceFan = std::make_shared<Source>(180, 60, 0.8);
	auto sourceBaby = std::make_shared<Source>(90, 50, 0.98);
	auto sourceCClock = std::make_shared<Source>(270, 0, 0.8);
	auto sourceFaucet = std::make_shared<Source>(170, 0, 0.8);

	/*auto soundCreackDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/creaking-door-2.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundOpenDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/door-2-open.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundCloseDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/door-2-close.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundLightSwitch = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/switch-1.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPeople = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/crowd-talking-1.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourcePeople, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPersonSteps = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/footsteps-1.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPersonStairs = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/wooden-stairs-1.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundFanSwitch = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/switch-19.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceHands, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundFan = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/fan.mp3"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceFan, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockWinding = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/clock-winding-1.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockTicking = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/clock-ticking-5.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockAlarm = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/alarm-clock-01.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundWashing = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/washing-dishes-2.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceFaucet, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundFaucet = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/water-faucet-1.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceFaucet, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundBabyCry = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/baby-whining-02.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceBaby, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundBabyLaugh = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/baby-laughing-04.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceBaby, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundSong = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/c.ogg"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceHands, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundAlarm = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<ConvolverSound>(std::make_shared<File>("effects/alarm-clock-01.wav"), rev_ir, threadPool, fftPlan), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);*/

	auto soundCreackDoor = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/creaking-door-2.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundOpenDoor = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/door-2-open.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundCloseDoor = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/door-2-close.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundLightSwitch = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/switch-1.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundPeople = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/crowd-talking-1.wav"), specs), hrtfs, sourcePeople, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundPersonSteps = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/footsteps-1.wav"), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundPersonStairs = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/wooden-stairs-1.wav"), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundFanSwitch = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/switch-19.wav"), specs), hrtfs, sourceHands, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundFan = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/fan.mp3"), specs), hrtfs, sourceFan, threadPool, fftPlan), i_ir, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockWinding = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/clock-winding-1.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundClockTicking = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/clock-ticking-5.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundClockAlarm = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/alarm-clock-01.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundWashing = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/washing-dishes-2.wav"), specs), hrtfs, sourceFaucet, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundFaucet = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/water-faucet-1.wav"), specs), hrtfs, sourceFaucet, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundBabyCry = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/baby-whining-02.wav"), specs), hrtfs, sourceBaby, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundBabyLaugh = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/baby-laughing-04.wav"), specs), hrtfs, sourceBaby, threadPool, fftPlan), rev_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundSong = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/c.ogg"), specs), hrtfs, sourceHands, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);
	auto soundAlarm = std::make_shared<ConvolverSound>(std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/alarm-clock-01.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan), rev_ir, threadPool, fftPlan);

	/*auto soundCreackDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/creaking-door-2.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundOpenDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/door-2-open.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundCloseDoor = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/door-2-close.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundLightSwitch = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/switch-1.wav"), specs), hrtfs, sourceDoor, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPeople = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/crowd-talking-1.wav"), specs), hrtfs, sourcePeople, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPersonSteps = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/footsteps-1.wav"), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundPersonStairs = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/wooden-stairs-1.wav"), specs), hrtfs, sourcePerson, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundFanSwitch = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/switch-19.wav"), specs), hrtfs, sourceHands, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundFan = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/fan.mp3"), specs), hrtfs, sourceFan, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockWinding = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/clock-winding-1.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockTicking = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/clock-ticking-5.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundClockAlarm = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/alarm-clock-01.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundWashing = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/washing-dishes-2.wav"), specs), hrtfs, sourceFaucet, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundFaucet = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/water-faucet-1.wav"), specs), hrtfs, sourceFaucet, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundBabyCry = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/baby-whining-02.wav"), specs), hrtfs, sourceBaby, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundBabyLaugh = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/baby-laughing-04.wav"), specs), hrtfs, sourceBaby, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundSong = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/c.ogg"), specs), hrtfs, sourceHands, threadPool, fftPlan), i_ir, threadPool, fftPlan);
	auto soundAlarm = std::make_shared<ConvolverSound>(std::make_shared<BinauralSound>(std::make_shared<ChannelMapper>(std::make_shared<File>("effects/alarm-clock-01.wav"), specs), hrtfs, sourceCClock, threadPool, fftPlan), i_ir, threadPool, fftPlan);*/

	std::shared_ptr<IHandle> handle1;
	std::shared_ptr<IHandle> handle2;
	std::shared_ptr<IHandle> handle3;
	std::shared_ptr<IHandle> handlePeople;
	std::shared_ptr<IHandle> handleClock;
	std::shared_ptr<IHandle> handleFaucet;
	std::shared_ptr<IHandle> handleBaby;

	handle1 = playSound(manager, soundOpenDoor);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	handle1 = playSound(manager, soundPersonSteps);
	handle2 = playSound(manager, soundCreackDoor, 0.2);
	handlePeople = playSound(manager, soundPeople, 0.07, -1);
	fadeInOut(handlePeople, 1.0, 3000);
	handle1->stop();
	handle2 = playSound(manager, soundCreackDoor, 0.2);
	fadeInOut(handlePeople, 0.07, 3000);
	playSound(manager, soundCloseDoor);
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	playSound(manager, soundLightSwitch);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	manager->clean();

	handle1 = playSound(manager, soundPersonSteps);
	moveSourceStraight(sourcePerson, 30, sourcePerson->getElevation(), -90, 0.35, 4300);
	moveSourceCircle(sourcePerson, 10, sourcePerson->getElevation(), 700);
	fadeInOut(handle1, 0, 500);
	handle1->stop();
	manager->clean();

	sourceHands->setAzimuth(sourcePerson->getAzimuth());
	sourceHands->setDistance(sourcePerson->getDistance());
	playSound(manager, soundFanSwitch);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	/*handle1 = playSound(manager, soundSong, 0.0, -1);
	fadeInOut(handle1, 0.1, 1000);*/
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	manager->clean();

	handle1 = playSound(manager, soundPersonSteps);
	moveSourceCircle(sourcePerson, -1, sourcePerson->getElevation(), 700);
	moveSourceStraight(sourcePerson, 270, sourcePerson->getElevation(), 60, 0.35, 3600);
	fadeInOut(handle1, 0, 500);
	handle1->stop();
	manager->clean();

	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	sourceCClock->setDistance(sourcePerson->getDistance() + 0.3);
	handle1 = playSound(manager, soundClockWinding);
	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
	handle1->stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	handleClock = playSound(manager, soundClockTicking, 0.5, -1);
	manager->clean();

	handle1 = playSound(manager, soundPersonSteps);
	moveSourceStraight(sourcePerson, 180, sourcePerson->getElevation(), 135, 0.35, 4000);
	fadeInOut(handle1, 0, 500);
	handle1->stop();
	manager->clean();

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	sourceFaucet->setDistance(sourcePerson->getDistance() + 0.25);
	handleFaucet = playSound(manager, soundWashing);
	std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	handleBaby = playSound(manager, soundBabyCry);
	fadeInOut(handleFaucet, 0, 1000);
	handleFaucet->stop();
	handleFaucet = playSound(manager, soundFaucet, 0.2);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	manager->clean();

	handle1 = playSound(manager, soundPersonSteps, 1, 0, 1.3);
	moveSourceStraight(sourcePerson, 90, sourcePerson->getElevation(), -135, 0.35, 3000);
	fadeInOut(handle1, 0.0, 500);
	handle1->stop();
	handle1 = playSound(manager, soundPersonStairs, 1, 0, 1.3);
	moveSourceStraight(sourcePerson, sourcePerson->getAzimuth(), 50, -135, 0.35, 3000);
	fadeInOut(handle1, 0.0, 500);
	handle1->stop();
	handle1 = playSound(manager, soundPersonSteps, 1, 0, 1.3);
	moveSourceAway(sourcePerson, 0.98, 1000);
	fadeInOut(handle1, 0.0, 500);
	handle1->stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	manager->clean();

	fadeInOut(handleBaby, 0.0, 500);
	handleBaby->stop();
	handleBaby = playSound(manager, soundBabyLaugh, 0.3);
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	manager->clean();

	handle1 = playSound(manager, soundPersonSteps, 1, 0, 1.0);
	moveSourceAway(sourcePerson, 0.77, 1500);
	fadeInOut(handle1, 0.0, 500);
	handle1->stop();
	handle1 = playSound(manager, soundPersonStairs, 1, 0, 1.0);
	moveSourceStraight(sourcePerson, sourcePerson->getAzimuth(), -40, -135, 0.35, 3700);
	fadeInOut(handle1, 0.0, 500);
	handle1->stop();
	handle1 = playSound(manager, soundPersonSteps, 1, 0, 1.0);
	moveSourceStraight(sourcePerson, 180, sourcePerson->getElevation(), -135, 0.35, 3500);
	fadeInOut(handle1, 0.0, 500);
	handle1->stop();
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	manager->clean();
	
	fadeInOut(handleFaucet, 0.0, 500);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	handleClock->stop();
	handleClock = playSound(manager, soundAlarm, 0.3);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	handleBaby = playSound(manager, soundBabyCry);
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	manager->clean();

	for(int i = 0;i < 40;i++)
	{
		float volume = manager->getVolume(0);
		volume -= 0.025;
		if(volume < 0.0f)
			volume = 0.0f;
		manager->setVolume(volume, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
	manager->stop(0);
	return 0;
}

std::shared_ptr<IHandle> playSound(std::shared_ptr<PlaybackManager> manager, std::shared_ptr<ISound> sound, float volume, int loopCount, float pitch, unsigned int category)
{
	manager->getDevice()->lock();
	auto handle = manager->play(sound, category);
	handle->setVolume(volume);
	handle->setPitch(pitch);
	handle->setLoopCount(loopCount);
	manager->getDevice()->unlock();
	return handle;
}

void fadeInOut(std::shared_ptr<IHandle> handle, float limit, int duration)
{
	int iterations = duration / 50;
	float step = (limit - handle->getVolume()) / iterations;
	for(int i = 0;i < iterations;i++)
	{
		float volume = handle->getVolume();
		volume += step;
		if(volume < 0.0f)
			volume = 0.0f;
		else if(volume > 1.0f)
			volume = 1.0f;
		handle->setVolume(volume);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void moveSourceStraight(std::shared_ptr<Source> source, float targetAzimuth, float targetElevation, float lineAngle, float minDistance, int duration)
{
	int iterations = duration / 100;
	float azStep = (targetAzimuth - source->getAzimuth()) / iterations;
	float elStep = (targetElevation - source->getElevation()) / iterations;
	for(int i = 0;i < iterations;i++)
	{
		source->setAzimuth(source->getAzimuth() + azStep);
		source->setElevation(source->getElevation() + elStep);
		source->setDistance(minDistance / abs(cos((source->getAzimuth() + lineAngle)*PI / 180.0)));
		source->setDistance(source->getDistance() / abs(cos(source->getElevation()*PI / 180.0)));
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void moveSourceCircle(std::shared_ptr<Source> source, float targetAzimuth, float targetElevation, int duration)
{
	int iterations = duration / 100;
	float azStep = (targetAzimuth - source->getAzimuth()) / iterations;
	float elStep = (targetElevation - source->getElevation()) / iterations;
	for(int i = 0;i < iterations;i++)
	{
		source->setAzimuth(source->getAzimuth() + azStep);
		source->setElevation(source->getElevation() + elStep);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void moveSourceAway(std::shared_ptr<Source> source, float targetDistance, int duration)
{
	int iterations = duration / 100;
	float step = (targetDistance - source->getDistance()) / iterations;
	for(int i = 0;i < iterations;i++)
	{
		source->setDistance(source->getDistance() + step);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void loadSounds(const std::string& path, const std::string& ext, std::vector<std::shared_ptr<ISound>>& outList)
{
	std::string readpath = path;
	if(path == "")
		readpath = ".";

	WIN32_FIND_DATA entry;
	bool found_file = true;
	std::string search = readpath + "\\*";
	HANDLE dir = FindFirstFile(search.c_str(), &entry);
	if(dir == INVALID_HANDLE_VALUE)
		return;

	float azim, elev;

	while(found_file)
	{
		std::string filename = entry.cFileName;
		if(filename.length() >= ext.length() && filename.substr(filename.length() - ext.length()) == ext)
			outList.push_back(std::make_shared<File>(readpath + "/" + filename));
		found_file = FindNextFile(dir, &entry);
	}
	FindClose(dir);
	return;
}
