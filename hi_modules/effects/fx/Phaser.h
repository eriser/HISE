/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/
#ifndef PHASER_H_INCLUDED
#define PHASER_H_INCLUDED


class PhaseFX: public MasterEffectProcessor
{
public:
    
    SET_PROCESSOR_NAME("PhaseFX", "Phase FX")
    
    enum Attributes
    {
        Frequency1 = 0,
        Frequency2,
        Feedback,
        Mix,
        numAttributes
    };
    
	enum InternalChains
	{
		PhaseModulationChain = 0,
		numInternalChains
	};

	enum EditorStates
	{
		PhaseModulationChainShown = Processor::numEditorStates,
		numEditorStates
	};

    PhaseFX(MainController *mc, const String &id);;
    
	~PhaseFX() {};

    float getAttribute(int parameterIndex) const override;;
    void setInternalAttribute(int parameterIndex, float newValue) override;;
    
    void restoreFromValueTree(const ValueTree &v) override;;
    ValueTree exportAsValueTree() const override;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;;
    void applyEffect(AudioSampleBuffer &buffer, int startSample, int numSamples) override;;
    
    bool hasTail() const override { return false; };
    int getNumChildProcessors() const override { return numInternalChains; };
	int getNumInternalChains() const override { return numInternalChains; };
    Processor *getChildProcessor(int /*processorIndex*/) override { return phaseModulationChain; };
    const Processor *getChildProcessor(int /*processorIndex*/) const override { return phaseModulationChain; };
    
	AudioSampleBuffer &getBufferForChain(int /*index*/) override { return phaseModulationBuffer; };

    ProcessorEditorBody *createEditor(ProcessorEditor *parentEditor)  override;
    
private:
    
	class PhaseModulator{
	public:
		PhaseModulator();

		void setRange(float freq1, float freq2);
		void setFeedback(float newFeedback) noexcept { feedback = 0.99f * newFeedback; }
		void setSampleRate(double newSampleRate);

		float getNextSample(float input, float modValue);;

	private:

		class AllpassDelay
		{
		public:
			AllpassDelay(): 
				delay(0.f),
				currentValue(0.f)
			{}

			static float getDelayCoefficient(float delaySamples) 
			{	
				return (1.f - delaySamples) / (1.f + delaySamples);
			}

			void setDelay(float newDelay) noexcept { delay = newDelay; };

			float getNextSample(float input) noexcept
			{
				float y = input * - delay + currentValue;
				currentValue = y * delay + input;

				return y;
			}

		private:
			float delay, currentValue;
		};

		AllpassDelay allpassFilters[6];

		float minDelay, maxDelay;
		float feedback;
		float sampleRate;
		float fMin;
		float fMax;
		float rate;
		
		float currentValue;
	};

	void updateFrequencies()
	{
		phaserLeft.setRange(freq1, freq2);
		phaserRight.setRange(freq1, freq2);
	}

	float freq1, freq2;
	float feedback;
	float mix;

	ScopedPointer<ModulatorChain> phaseModulationChain;

	AudioSampleBuffer phaseModulationBuffer;

	PhaseModulator phaserLeft;
	PhaseModulator phaserRight;
};

#endif  // PHASER_H_INCLUDED
