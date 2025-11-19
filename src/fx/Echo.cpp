#include "fx/Echo.h"

#include "fx/EchoReader.h"

AUD_NAMESPACE_BEGIN

Echo::Echo(std::shared_ptr<ISound> sound, float delay, float feedback, float mix, bool resetBuffer) :
    Effect(sound), m_delay(delay), m_feedback(feedback), m_mix(mix), m_resetBuffer(resetBuffer) {}

float Echo::getDelay() const { return m_delay; }
float Echo::getFeedback() const { return m_feedback; }
float Echo::getMix() const { return m_mix; }
bool Echo::getResetBuffer() const { return m_resetBuffer; }

void Echo::setDelay(float delay) {
    if(delay > 0){
        m_delay = delay; 
    }
}
void Echo::setFeedback(float feedback) {
    if(feedback >= 0 && feedback <= 1){
        m_feedback = feedback; 
    }
}
void Echo::setMix(float mix) {
    if(mix >= 0 && mix <= 1){
        m_mix = mix; 
    }
}
void Echo::setResetBuffer(bool resetBuffer) {
    m_resetBuffer = resetBuffer;
}

std::shared_ptr<IReader> Echo::createReader()
{
    return std::make_shared<EchoReader>(getReader(), m_delay, m_feedback, m_mix, m_resetBuffer);
}

AUD_NAMESPACE_END