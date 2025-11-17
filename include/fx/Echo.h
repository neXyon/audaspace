#pragma once
/**
 * @file Echo.h
 * @ingroup fx
 * The Echo class.
 */
#include "fx/Effect.h"
#include <vector>
AUD_NAMESPACE_BEGIN
class AUD_API Echo : public Effect
{
private:
    double m_delaySec;      /* Delay time in seconds */
    double m_feedback;      /* Feedback amount */
    double m_mix;           /* Wet/dry mix */

public:
    Echo(std::shared_ptr<ISound> sound, double delaySec, double feedback, double mix);

    double getDelay() const;
    double getFeedback() const;
    double getMix() const;

    void setDelay(double delaySec);
    void setFeedback(double feedback);
    void setMix(double mix);
    
    virtual std::shared_ptr<IReader> createReader();
};

AUD_NAMESPACE_END