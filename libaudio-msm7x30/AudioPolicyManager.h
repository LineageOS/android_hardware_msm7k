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


#include <stdint.h>
#include <sys/types.h>
#include <utils/Timers.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <hardware_legacy/AudioPolicyManagerBase.h>


namespace android {

class AudioPolicyManager: public AudioPolicyManagerBase
{

public:
#ifdef USE_BROADCOM_FM_VOLUME_HACK
                AudioPolicyManager(AudioPolicyClientInterface *clientInterface);
#else
                AudioPolicyManager(AudioPolicyClientInterface *clientInterface)
                : AudioPolicyManagerBase(clientInterface) {}
#endif

        virtual ~AudioPolicyManager() {}

#ifdef USE_BROADCOM_FM_VOLUME_HACK
        virtual void setOutputDevice(audio_io_handle_t output, uint32_t device, bool force = false, int delayMs = 0);
#endif

protected:
        // true is current platform implements a back microphone
        virtual bool hasBackMicrophone() const { return false; }
#ifdef WITH_A2DP
        // true is current platform supports suplication of notifications and ringtones over A2DP output
        virtual bool a2dpUsedForSonification() const { return true; }
#endif

#ifdef USE_BROADCOM_FM_VOLUME_HACK
        enum fmradio_device_t {
            FMRADIO_NONE,
            FMRADIO_HANDSET_RX,
            FMRADIO_SPEAKER_RX,
            FMRADIO_COUNT
        };

        fmradio_device_t mFmDevice;
        unsigned mFmDeviceIds[FMRADIO_COUNT];

        void setFmState(bool on, fmradio_device_t device);
#endif
};
};
