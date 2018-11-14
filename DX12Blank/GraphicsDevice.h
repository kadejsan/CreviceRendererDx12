#pragma once

#include "GraphicsDescriptors.h"

class BaseWindow;
class GPUResource;

class GraphicsDevice
{
protected:
	static const UINT32 st_frameCount = 2; // double buffering
	static const UINT   st_useVsync = 0;

public:
	GraphicsDevice()
		: m_isInitialized( false )
		, m_frameIndex( 0 )
	{}
	virtual ~GraphicsDevice() {};

	virtual void Initialize(BaseWindow* window) = 0;

	virtual void Flush() = 0;
	virtual HANDLE GetFenceEvent() const = 0;

	// Gpu Api Interface
	virtual void PresentBegin() = 0;
	virtual void PresentEnd() = 0;

	virtual void TransitionBarrier( GPUResource* resources, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter ) = 0;
	virtual void TransitionBarriers(GPUResource* const* resources, UINT NumBarriers, RESOURCE_STATES stateBefore, RESOURCE_STATES stateAfter) = 0;

	inline bool IsInitialized() const { return m_isInitialized; }
	inline UINT32 GetCurrentFrameIndex() const { return m_frameIndex; }
	
protected:
	bool								m_isInitialized;
	UINT32								m_frameIndex;
};