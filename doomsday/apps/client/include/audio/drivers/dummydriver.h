/** @file dummydriver.cpp  Dummy audio driver for simulating playback.
 *
 * @authors Copyright © 2003-2013 Jaakko Keränen <jaakko.keranen@iki.fi>
 * @authors Copyright © 2007-2015 Daniel Swanson <danij@dengine.net>
 *
 * @par License
 * LGPL: http://www.gnu.org/licenses/lgpl.html
 *
 * <small>This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version. This program is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 * General Public License for more details. You should have received a copy of
 * the GNU Lesser General Public License along with this program; if not, see:
 * http://www.gnu.org/licenses</small>
 */

#ifndef CLIENT_AUDIO_DUMMYDRIVER_H
#define CLIENT_AUDIO_DUMMYDRIVER_H

#ifdef __SERVER__
#  error "audio" is not available in a SERVER build
#endif

#include "audio/channel.h"
#include "audio/idriver.h"
#include <de/String>

namespace audio {

class DummyCdChannel : public CdChannel
{
public:
    DummyCdChannel();

    PlayingMode mode() const override;
    void play(PlayingMode mode) override;
    void stop() override;
    bool isPaused() const override;
    void pause() override;
    void resume() override;
    Channel &setFrequency(de::dfloat newFrequency) override;
    Channel &setPositioning(Positioning newPositioning) override;
    Channel &setVolume(de::dfloat newVolume) override;
    de::dfloat frequency() const override;
    Positioning positioning() const override;
    de::dfloat volume() const override;

    void bindTrack(de::dint track);

private:
    PlayingMode _mode     = NotPlaying;
    bool _paused          = false;
    de::dint _track       = -1;
    de::dfloat _frequency = 1;
    de::dfloat _volume    = 1;
};

class DummyMusicChannel : public MusicChannel
{
public:
    DummyMusicChannel();

    PlayingMode mode() const override;
    void play(PlayingMode mode) override;
    void stop() override;
    bool isPaused() const override;
    void pause() override;
    void resume() override;
    Channel &setFrequency(de::dfloat newFrequency) override;
    Channel &setPositioning(Positioning newPositioning) override;
    Channel &setVolume(de::dfloat newVolume) override;
    de::dfloat frequency() const override;
    Positioning positioning() const override;
    de::dfloat volume() const override;

    bool canPlayBuffer() const;
    void *songBuffer(de::duint length);
    bool canPlayFile() const;
    void bindFile(de::String const &sourcePath);

private:
    PlayingMode _mode      = NotPlaying;
    bool _paused           = false;
    de::String _sourcePath;
    de::dfloat _frequency  = 1;
    de::dfloat _volume     = 1;
};

class DummySoundChannel : public SoundChannel
{
public:
    DummySoundChannel();

    PlayingMode mode() const override;
    void play(PlayingMode mode) override;
    void stop() override;
    bool isPaused() const override;
    void pause() override;
    void resume() override;
    Channel &setFrequency(de::dfloat newFrequency) override;
    Channel &setPositioning(Positioning newPositioning) override;
    Channel &setVolume(de::dfloat newVolume) override;
    de::dfloat frequency() const override;
    Positioning positioning() const override;
    de::dfloat volume() const override;

    ::audio::Sound *sound() const override;

    void update();
    void reset();
    void suspend() override;

    void bindSample(sfxsample_t const &sample) override;

    de::dint bytes() const;
    de::dint rate() const;
    de::dint startTime() const override;
    de::duint endTime() const override;

    void updateEnvironment();

private:
    DENG2_PRIVATE(d)
};

/**
 * Provides a null-op audio driver.
 */
class DummyDriver : public IDriver
{
public:
    DummyDriver();
    virtual ~DummyDriver();

    void initialize();
    void deinitialize();

    Status status() const;

    de::String identityKey() const;
    de::String title() const;

    IChannelFactory &channelFactory() const override;

    de::LoopResult forAllChannels(Channel::Type type,
        std::function<de::LoopResult (Channel const &)> callback) const override;

    void allowRefresh(bool allow) override;

private:
    DENG2_PRIVATE(d)
};

}  // namespace audio

#endif  // CLIENT_AUDIO_DUMMYDRIVER_H
