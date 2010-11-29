/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "AudioPolicyManager"
//#define LOG_NDEBUG 0

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

extern "C" {
#include <linux/msm_audio_7X30.h>
}

#include <utils/Log.h>
#include "AudioPolicyManager.h"
#include <media/mediarecorder.h>

namespace android {

#ifdef USE_BROADCOM_FM_VOLUME_HACK
void AudioPolicyManager::AudioPolicyManager(AudioPolicyClientInterface *clientInterface)
    :
    AudioPolicyManagerBase(clientInterface),
    mFmDevice(FMRADIO_NONE)
{
    unsigned devices;
    int ret;

    mFmDeviceIds[FMRADIO_NONE] = -1;
    mFmDeviceIds[FMRADIO_HEADSET_RX] = -1;
    mFmDeviceIds[FMRADIO_SPEAKER_RX] = -1;

    int fd = open("/dev/msm_audio_dev_ctrl", O_RDONLY);
    if (fd < 0)
        goto err;

    ret = ioctl(fd, AUDIO_GET_NUM_SND_DEVICE, &devices);
    if (ret)
        goto err_close;

    struct msm_snd_device_info info[devices];
    struct msm_snd_device_list list = {
        .num_dev = devices,
        .list = info,
    };

    ret = ioctl(fd, AUDIO_GET_SND_DEVICES, &list);
    if (ret)
        goto err_close;

    close(fd);

    for (unsigned i = 0; i < list.num_dev; ++i) {
        if (!strcmp(info[i].dev_name, "fmradio_headset_rx"))
            mFmDeviceIds[FMRADIO_HEADSET_RX] = info[i].dev_id;
        else if (!strcmp(info[i].dev_name, "fmradio_speaker_rx"))
            mFmDeviceIds[FMRADIO_SPEAKER_RX] = info[i].dev_id;
    }

    if (mFmDeviceIds[FMRADIO_HEADSET_RX] < 0 ||
            mFmDeviceIds[FMRADIO_SPEAKER_RX] < 0) {
        goto err;
    }

    return;

err_close:
    close(fd);
err:
    LOGE("Error resolving FM device ids\n");
    return;
}

void AudioPolicyManager::setFmState(bool on, fmradio_device_t device)
{
    /* FIXME: Figure out how to do this using libaudio */
    if (device < 0) {
        LOGE("Attempted to set state of unresolved FM device\n");
        return;
    }

    int fd = open("/dev/msm_audio_dev_ctrl", O_WRONLY);
    if (fd < 0) {
        LOGE("Unable to open /dev/msm_audio_dev_ctrl: %s\n", strerror(errno));
        return;
    }

    ioctl(fd, on ? AUDIO_ENABLE_SND_DEVICE : AUDIO_DISABLE_SND_DEVICE, &mFmDeviceIds[device]);
    close(fd);
}

void AudioPolicyManager::setOutputDevice(audio_io_handle_t output, uint32_t device, bool force = false, int delayMs = 0)
{
    AudioPolicyManagerBase::setOutputDevice(output, device, force, delayMs);

    /* Figure out where we should route the FM radio */
    fmradio_device_t fmdev = FMRADIO_NONE;
    AudioOutputDescriptor *hwOutputDesc = mOutputs.valueFor(mHardwareOutput);
    if (hwOutputDesc->mRefCount[AudioSystem::FM]) {
        device = getDeviceForStrategy(STRATEGY_MEDIA);

        switch (device) {
            case AudioSystem::DEVICE_OUT_WIRED_HEADPHONE:
            case AudioSystem::DEVICE_OUT_WIRED_HEADSET:
                fmdev = FMRADIO_HEADSET_RX;
                break;
    
            case AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP:
            case AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
            case AudioSystem::DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
                /* We can't output FM radio directly to bluetooth. */
                fmdev = FMRADIO_SPEAKER_RX;
                break;
    
            case AudioSystem::DEVICE_OUT_EARPIECE:
                /* We can't output FM radio directly to the earpiece. */
                fmdev = FMRADIO_SPEAKER_RX;
                break;
    
            case AudioSystem::DEVICE_OUT_SPEAKER:
                fmdev = FMRADIO_SPEAKER_RX;
                break;
    
            default:
                setFmState(false, mFmDevice);
                mFmDevice = FMRADIO_NONE;
                return;
        }
    }

    /* Turn off/on the correct device(s), if needed */
    if (fmdev != mFmDevice) {
        if (mFmDevice)
            setFmState(false, mFmDevice);
        if (fmdev)
            setFmState(true, fmdev);
        mFmDevice = fmdev;
    }
}
#endif

// ----------------------------------------------------------------------------
// AudioPolicyManager for msm7k platform
// Common audio policy manager code is implemented in AudioPolicyManagerBase class
// ----------------------------------------------------------------------------

// ---  class factory


extern "C" AudioPolicyInterface* createAudioPolicyManager(AudioPolicyClientInterface *clientInterface)
{
    return new AudioPolicyManager(clientInterface);
}

extern "C" void destroyAudioPolicyManager(AudioPolicyInterface *interface)
{
    delete interface;
}

}; // namespace android
