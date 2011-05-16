/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEB_AUDIO)

#include "DynamicsCompressorKernel.h"

#include "AudioUtilities.h"
#include <algorithm>
#include <wtf/MathExtras.h>

using namespace std;

namespace WebCore {

using namespace AudioUtilities;

// Metering hits peaks instantly, but releases this fast (in seconds).
const double meteringReleaseTimeConstant = 0.325;
    
// Exponential saturation curve.
static double saturate(double x, double k)
{
    return 1 - exp(-k * x);
}

DynamicsCompressorKernel::DynamicsCompressorKernel(double sampleRate)
    : m_sampleRate(sampleRate)
    , m_lastPreDelayFrames(DefaultPreDelayFrames)
    , m_preDelayBufferL(MaxPreDelayFrames)
    , m_preDelayBufferR(MaxPreDelayFrames)
    , m_preDelayReadIndex(0)
    , m_preDelayWriteIndex(DefaultPreDelayFrames)
{
    // Initializes most member variables
    reset();
    
    m_meteringReleaseK = discreteTimeConstantForSampleRate(meteringReleaseTimeConstant, sampleRate);
}

void DynamicsCompressorKernel::setPreDelayTime(float preDelayTime)
{
    // Re-configure look-ahead section pre-delay if delay time has changed.
    unsigned preDelayFrames = preDelayTime / sampleRate();
    if (preDelayFrames > MaxPreDelayFrames - 1)
        preDelayFrames = MaxPreDelayFrames - 1;
        
    if (m_lastPreDelayFrames != preDelayFrames) {
        m_lastPreDelayFrames = preDelayFrames;
        m_preDelayBufferL.zero();
        m_preDelayBufferR.zero();
        m_preDelayReadIndex = 0;
        m_preDelayWriteIndex = preDelayFrames;
    }
}

void DynamicsCompressorKernel::process(float* sourceL,
                                       float* destinationL,
                                       float* sourceR, /* stereo-linked */
                                       float* destinationR,
                                       unsigned framesToProcess,

                                       float dbThreshold,
                                       float dbHeadroom,
                                       float attackTime,
                                       float releaseTime,
                                       float preDelayTime,
                                       float dbPostGain,
                                       float effectBlend, /* equal power crossfade */

                                       float releaseZone1,
                                       float releaseZone2,
                                       float releaseZone3,
                                       float releaseZone4
                                       )
{
    bool isStereo = destinationR;
    float sampleRate = this->sampleRate();

    float dryMix = 1 - effectBlend;
    float wetMix = effectBlend;

    // Threshold and headroom.
    double linearThreshold = decibelsToLinear(dbThreshold);
    double linearHeadroom = decibelsToLinear(dbHeadroom);

    // Makeup gain.
    double maximum = 1.05 * linearHeadroom * linearThreshold;
    double kk = (maximum - linearThreshold);
    double inverseKK = 1 / kk;

    double fullRangeGain = (linearThreshold + kk * saturate(1 - linearThreshold, 1));
    double fullRangeMakeupGain = 1 / fullRangeGain;
    // Empirical/perceptual tuning.
    fullRangeMakeupGain = pow(fullRangeMakeupGain, 0.6);

    float masterLinearGain = decibelsToLinear(dbPostGain) * fullRangeMakeupGain;

    // Attack parameters.
    attackTime = max(0.001f, attackTime);
    float attackFrames = attackTime * sampleRate;

    // Release parameters.
    float releaseFrames = sampleRate * releaseTime;
    
    // Detector release time.
    double satReleaseTime = 0.0025;
    double satReleaseFrames = satReleaseTime * sampleRate;

    // Create a smooth function which passes through four points.

    // Polynomial of the form
    // y = a + b*x + c*x^2 + d*x^3 + e*x^4;

    double y1 = releaseFrames * releaseZone1;
    double y2 = releaseFrames * releaseZone2;
    double y3 = releaseFrames * releaseZone3;
    double y4 = releaseFrames * releaseZone4;

    // All of these coefficients were derived for 4th order polynomial curve fitting where the y values
    // match the evenly spaced x values as follows: (y1 : x == 0, y2 : x == 1, y3 : x == 2, y4 : x == 3)
    double kA = 0.9999999999999998*y1 + 1.8432219684323923e-16*y2 - 1.9373394351676423e-16*y3 + 8.824516011816245e-18*y4;
    double kB = -1.5788320352845888*y1 + 2.3305837032074286*y2 - 0.9141194204840429*y3 + 0.1623677525612032*y4;
    double kC = 0.5334142869106424*y1 - 1.272736789213631*y2 + 0.9258856042207512*y3 - 0.18656310191776226*y4;
    double kD = 0.08783463138207234*y1 - 0.1694162967925622*y2 + 0.08588057951595272*y3 - 0.00429891410546283*y4;
    double kE = -0.042416883008123074*y1 + 0.1115693827987602*y2 - 0.09764676325265872*y3 + 0.028494263462021576*y4;

    // x ranges from 0 -> 3       0    1    2   3
    //                           -15  -10  -5   0db

    // y calculates adaptive release frames depending on the amount of compression.

    setPreDelayTime(preDelayTime);
    
    const int nDivisionFrames = 32;

    const int nDivisions = framesToProcess / nDivisionFrames;

    for (int i = 0; i < nDivisions; ++i) {
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Calculate desired gain
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // Fix gremlins.
        if (isnan(m_detectorAverage))
            m_detectorAverage = 1;
        if (isinf(m_detectorAverage))
            m_detectorAverage = 1;

        float desiredGain = m_detectorAverage;

        // Pre-warp so we get desiredGain after sin() warp below.
        double scaledDesiredGain = asin(desiredGain) / (0.5 * piDouble);

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Deal with envelopes
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        // envelopeRate is the rate we slew from current compressor level to the desired level.
        // The exact rate depends on if we're attacking or releasing and by how much.
        float envelopeRate;

        bool isReleasing = scaledDesiredGain > m_compressorGain;

        // compressionDiffDb is the difference between current compression level and the desired level.
        double compressionDiffDb = linearToDecibels(m_compressorGain / scaledDesiredGain);

        if (isReleasing) {
            // Release mode - compressionDiffDb should be negative dB
            m_maxAttackCompressionDiffDb = -1;

            // Fix gremlins.
            if (isnan(compressionDiffDb))
                compressionDiffDb = -1;
            if (isinf(compressionDiffDb))
                compressionDiffDb = -1;

            // Adaptive release - higher compression (lower compressionDiffDb)  releases faster.

            // Contain within range: -12 -> 0 then scale to go from 0 -> 3
            double x = compressionDiffDb;
            x = max(-12., x);
            x = min(0., x);
            x = 0.25 * (x + 12);

            // Compute adaptive release curve using 4th order polynomial.
            // Normal values for the polynomial coefficients would create a monotonically increasing function.
            double x2 = x * x;
            double x3 = x2 * x;
            double x4 = x2 * x2;
            double releaseFrames = kA + kB * x + kC * x2 + kD * x3 + kE * x4;

#define kSpacingDb 5
            double dbPerFrame = kSpacingDb / releaseFrames;

            envelopeRate = decibelsToLinear(dbPerFrame);
        } else {
            // Attack mode - compressionDiffDb should be positive dB

            // Fix gremlins.
            if (isnan(compressionDiffDb))
                compressionDiffDb = 1;
            if (isinf(compressionDiffDb))
                compressionDiffDb = 1;

            // As long as we're still in attack mode, use a rate based off
            // the largest compressionDiffDb we've encountered so far.
            if (m_maxAttackCompressionDiffDb == -1 || m_maxAttackCompressionDiffDb < compressionDiffDb)
                m_maxAttackCompressionDiffDb = compressionDiffDb;

            double effAttenDiffDb = max(0.5f, m_maxAttackCompressionDiffDb);

            double x = 0.25 / effAttenDiffDb;
            envelopeRate = 1 - pow(x, double(1 / attackFrames));
        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Inner loop - calculate shaped power average - apply compression.
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        {
            float* delayBufferL = m_preDelayBufferL.data();
            float* delayBufferR = m_preDelayBufferR.data();
            int preDelayReadIndex = m_preDelayReadIndex;
            int preDelayWriteIndex = m_preDelayWriteIndex;
            float detectorAverage = m_detectorAverage;
            float compressorGain = m_compressorGain;

            int loopFrames = nDivisionFrames;
            while (loopFrames--) {
                float compressorInput;
                float inputL;
                float inputR = 0;

                // Predelay signal, computing compression amount from un-delayed version.
                if (isStereo) {
                    float undelayedL = *sourceL++;
                    float undelayedR = *sourceR++;

                    compressorInput = 0.5 * (undelayedL + undelayedR);

                    inputL = delayBufferL[preDelayReadIndex];
                    inputR = delayBufferR[preDelayReadIndex];

                    delayBufferL[preDelayWriteIndex] = undelayedL;
                    delayBufferR[preDelayWriteIndex] = undelayedR;
                } else {
                    compressorInput = *sourceL++;

                    inputL = delayBufferL[preDelayReadIndex];
                    delayBufferL[preDelayWriteIndex] = compressorInput;
                }

                preDelayReadIndex = (preDelayReadIndex + 1) & MaxPreDelayFramesMask;
                preDelayWriteIndex = (preDelayWriteIndex + 1) & MaxPreDelayFramesMask;

                // Calculate shaped power on undelayed input.

                float scaledInput = compressorInput;
                double absInput = scaledInput > 0 ? scaledInput : -scaledInput;

                // Put through shaping curve.
                // This is linear up to the threshold, then exponentially approaches the maximum (headroom amount above threshold).
                // The transition from the threshold to the exponential portion is smooth (1st derivative matched).
                double shapedInput = absInput < linearThreshold ? absInput : linearThreshold + kk * saturate(absInput - linearThreshold, inverseKK);

                double attenuation = absInput <= 0.0001 ? 1 : shapedInput / absInput;

                double attenuationDb = -linearToDecibels(attenuation);
                attenuationDb = max(2., attenuationDb);

                double dbPerFrame = attenuationDb / satReleaseFrames;

                double satReleaseRate = decibelsToLinear(dbPerFrame) - 1;

                bool isRelease = (attenuation > detectorAverage);
                double rate = isRelease ? satReleaseRate : 1;

                detectorAverage += (attenuation - detectorAverage) * rate;
                detectorAverage = min(1.0f, detectorAverage);

                // Fix gremlins.
                if (isnan(detectorAverage))
                    detectorAverage = 1;
                if (isinf(detectorAverage))
                    detectorAverage = 1;

                // Exponential approach to desired gain.
                if (envelopeRate < 1) {
                    // Attack - reduce gain to desired.
                    compressorGain += (scaledDesiredGain - compressorGain) * envelopeRate;
                } else {
                    // Release - exponentially increase gain to 1.0
                    compressorGain *= envelopeRate;
                    compressorGain = min(1.0f, compressorGain);
                }

                // Warp pre-compression gain to smooth out sharp exponential transition points.
                double postWarpCompressorGain = sin(0.5 * piDouble * compressorGain);

                // Calculate total gain using master gain and effect blend.
                double totalGain = dryMix + wetMix * masterLinearGain * postWarpCompressorGain;

                // Calculate metering.
                double dbRealGain = 20 * log10(postWarpCompressorGain);
                if (dbRealGain < m_meteringGain)
                    m_meteringGain = dbRealGain;
                else
                    m_meteringGain += (dbRealGain - m_meteringGain) * m_meteringReleaseK;

                // Apply final gain.
                if (isStereo) {
                    float outputL = inputL;
                    float outputR = inputR;

                    outputL *= totalGain;
                    outputR *= totalGain;

                    *destinationL++ = outputL;
                    *destinationR++ = outputR;
                } else
                    *destinationL++ = inputL * totalGain;
            }

            // Locals back to member variables.
            m_preDelayReadIndex = preDelayReadIndex;
            m_preDelayWriteIndex = preDelayWriteIndex;
            m_detectorAverage = detectorAverage;
            m_compressorGain = compressorGain;
        }
    }
}

void DynamicsCompressorKernel::reset()
{
    m_detectorAverage = 0;
    m_compressorGain = 1;
    m_meteringGain = 1;

    // Predelay section.
    m_preDelayBufferL.zero();
    m_preDelayBufferR.zero();
    m_preDelayReadIndex = 0;
    m_preDelayWriteIndex = DefaultPreDelayFrames;

    m_maxAttackCompressionDiffDb = -1; // uninitialized state
}

} // namespace WebCore

#endif // ENABLE(WEB_AUDIO)
