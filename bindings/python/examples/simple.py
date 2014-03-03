import aud
device = aud.Device('OpenAL')
sine = aud.Sound.sine(440)
square = sine.square()
handle = device.play(square)
