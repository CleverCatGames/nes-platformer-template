# Sound Library

This contains the `famistudio_ca65.s` code, unmodified.

`music.s` and `sfx.s` are both generated from FamiStudio. Do not attempt to create these by hand. See `assets/sound/music.fms` and `assets/sound/sfx.fms` for their respective FamiStudio files.

`sound.s` is where everything gets pulled together. It gives C several useful functions like the following:

* music_play - plays the specified song ID, if it isn't already playing
* music_pause/music_resume - pause and resume music that is playing
* sound_play_ch0/1 - plays a sound on one of the two different channels
* music_save/music_restore - this lets you play a jingle and then restore to the previous playing music. I've used it to play a powerup "song"

