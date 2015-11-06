#pragma once

/**
* @file HRTF.h
* @ingroup fx
* The HRTF class.
*/

#include "util/StreamBuffer.h"
#include "util/FFTPlan.h"
#include "ImpulseResponse.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>

AUD_NAMESPACE_BEGIN

/**
*
*/
class AUD_API HRTF
{
private:
	/**
	*
	*/
	std::unordered_map<float, std::unordered_map<float, std::shared_ptr<ImpulseResponse>>> m_hrtfs;

	/**
	*
	*/
	std::shared_ptr<FFTPlan> m_plan;

	/**
	*
	*/
	Specs m_specs;

	// delete copy constructor and operator=
	HRTF(const HRTF&) = delete;
	HRTF& operator=(const HRTF&) = delete;

public:
	/**
	*
	*/
	HRTF();

	/**
	*
	*/
	HRTF(std::shared_ptr<FFTPlan> plan);

	/**
	*
	* \exception
	*/
	bool addImpulseResponse(std::shared_ptr<StreamBuffer> impulseResponse, float azimuth, float elevation);

	/**
	*
	*/
	std::pair<std::shared_ptr<ImpulseResponse>, std::shared_ptr<ImpulseResponse>> getImpulseResponse(float &azimuth, float &elevation);

	/**
	*
	*/
	Specs getSpecs();
};

AUD_NAMESPACE_END
