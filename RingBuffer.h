#pragma once

class RingBuffer
{
public:
	RingBuffer(void);
	RingBuffer(int BufferSize);
	RingBuffer(const RingBuffer&) = delete;
	RingBuffer& operator=(const RingBuffer&) = delete;
	~RingBuffer(void);

	int GetBufferSize(void) const noexcept;
	int GetUseSize(void) const noexcept;
	int GetFreeSize(void) const noexcept;

	int Enqueue(const char* Data, int Size) noexcept;
	int Dequeue(char* Dest, int Size) noexcept;
	int Peek(char* Dest, int Size) const noexcept;

	void ClearBuffer(void) noexcept;

	int DirectEnqueueSize(void) const noexcept;
	int DirectDequeueSize(void) const noexcept;

	int MoveRear(int Size) noexcept;
	int MoveFront(int Size) noexcept;

	char* GetFrontBufferPtr(void) const noexcept;
	char* GetRearBufferPtr(void) const noexcept;

private:
	char* _buffer;
	int _buffersize;
	char* _front;
	char* _rear;
};

