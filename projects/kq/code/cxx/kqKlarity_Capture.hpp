#include "memory"

struct IAudioCaptureEngine
{
	static std::shared_ptr<IAudioCaptureEngine> createEngine();
	virtual uint32_t getSamplingRate() = 0;
	virtual uint32_t getBitsPerSample() = 0;
	virtual uint32_t getChannelCount() = 0;

	virtual bool start() = 0;
	virtual bool waitForData() = 0;
	virtual bool areChannelsInterleaved() = 0;
	virtual bool stop() = 0;
	virtual 
};