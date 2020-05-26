/*******************************************************************************
 * Copyright 2009-2016 Jörg Müller
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include "devices/InterpolatedSynchronizer.h"
#include "devices/IHandle.h"
#include <chrono>
#include <cmath>

AUD_NAMESPACE_BEGIN

void InterpolatedSynchronizer::seek(std::shared_ptr<IHandle> handle, double time)
{
	reset_time_interpolator = true;
	handle->seek(time);
}

double InterpolatedSynchronizer::getPosition(std::shared_ptr<IHandle> handle)
{
	double new_pos = handle->getPosition();
	if (!is_playing) {
		return new_pos;
	}

	using namespace std::chrono;

	if (reset_time_interpolator) {
		t1 = steady_clock::now();
		offset = 0;
		adjust = 0;
		prev_pos = new_pos;
		reset_time_interpolator = false;
	}
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

	if (prev_pos == new_pos) {
		// The new position is the exact same as the last one despite us playing back sound!
		// Interpolate the next position to get a somewhat constant time flow.
		offset += time_span.count() + adjust;
	} else {
		double time_step = (new_pos - prev_pos);

		offset = offset - time_step;
		offset += time_span.count() + adjust;

		if (std::abs(offset) > 0.02f) {
			// Try to minimize the offset by slowly drifting towards the minimal error margin.
			adjust = 0.001f;
			adjust = std::copysign(adjust,offset);
			adjust *= -1.0f;
		} else {
			adjust = 0;
		}

		prev_pos = new_pos;
	}

	t1 = steady_clock::now();

	new_pos += offset;

	return new_pos;
}

void InterpolatedSynchronizer::play()
{
	reset_time_interpolator = true;
	is_playing = true;
}

void InterpolatedSynchronizer::stop()
{
	is_playing = false;
}

void InterpolatedSynchronizer::setSyncCallback(ISynchronizer::syncFunction function, void* data)
{
}

int InterpolatedSynchronizer::isPlaying()
{
	return -1;
}

AUD_NAMESPACE_END
