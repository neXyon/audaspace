#include "fx/Echo.h"
#include "fx/EchoReader.h"

AUD_NAMESPACE_BEGIN

Echo::Echo(std::shared_ptr<ISound> sound, double delaySec, double feedback, double mix) :
    Effect(sound), m_delaySec(delaySec), m_feedback(feedback), m_mix(mix) {}

double Echo::getDelay() const { return m_delaySec; }
double Echo::getFeedback() const { return m_feedback; }
double Echo::getMix() const { return m_mix; }

void Echo::setDelay(double delaySec) {
    if(delaySec > 0){
        m_delaySec = delaySec; 
    }
}
void Echo::setFeedback(double feedback) {
    if(feedback >= 0 && feedback <= 1){
        m_feedback = feedback; 
    }
}
void Echo::setMix(double mix) {
    if(mix >= 0 && mix <= 1){
        m_mix = mix; 
    }
}

std::shared_ptr<IReader> Echo::createReader()
{
    return std::make_shared<EchoReader>(getReader(), m_delaySec, m_feedback, m_mix);
}

AUD_NAMESPACE_END