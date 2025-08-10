#include "RingBuffer.h"
#include <memory.h>
#include <malloc.h>

RingBuffer::RingBuffer(void) : _buffersize(100)
{
	_buffer = (char*)malloc(_buffersize);
	_front = _buffer;
	_rear = _buffer;
}

RingBuffer::RingBuffer(int BufferSize) : _buffersize(BufferSize)
{
	_buffer = (char*)malloc(_buffersize);
	_front = _buffer;
	_rear = _buffer;
}

RingBuffer::~RingBuffer(void)
{
	free(_buffer);
}

int RingBuffer::GetBufferSize(void) const noexcept
{
	return _buffersize;
}

int RingBuffer::GetUseSize(void) const noexcept
{
	if (_front <= _rear) return (int)(_rear - _front);
	else return (int)(_buffersize - (_front - _rear));
}

int RingBuffer::GetFreeSize(void) const noexcept
{
	return _buffersize - GetUseSize() - 1;
}

int RingBuffer::Enqueue(const char* Data, int Size) noexcept
{
	if (GetFreeSize() < Size) return 0;
	int firstPartSize = DirectEnqueueSize();
	if (firstPartSize >= Size) memcpy(_rear, Data, Size);
	else
	{
		memcpy(_rear, Data, firstPartSize);
		memcpy(_buffer, Data + firstPartSize, Size - firstPartSize);
	}
	MoveRear(Size);
	return Size;
}

int RingBuffer::Dequeue(char* Dest, int Size) noexcept
{
	int rs = Peek(Dest, Size);
	if (rs) MoveFront(Size);
	return rs;
}

int RingBuffer::Peek(char* Dest, int Size) const noexcept
{
	if (GetUseSize() < Size) return 0;
	int firstPartSize = DirectDequeueSize();
	if (firstPartSize >= Size) memcpy(Dest, _front, Size);
	else
	{
		memcpy(Dest, _front, firstPartSize);
		memcpy(Dest + firstPartSize, _buffer, Size - firstPartSize);
	}

	return Size;
}

void RingBuffer::ClearBuffer(void) noexcept
{
	_front = _buffer;
	_rear = _buffer;
}

int RingBuffer::DirectEnqueueSize(void) const noexcept
{
	if (_front <= _rear)
		return _buffersize - (_rear - _buffer) - (_front == _buffer ? 1 : 0);
	else
		return (int)(_front - _rear) - 1;
}

int RingBuffer::DirectDequeueSize(void) const noexcept
{
	if (_front <= _rear)
		return (int)(_rear - _front);
	else
		return _buffersize - (_front - _buffer);
}

int RingBuffer::MoveRear(int Size) noexcept
{
	if (GetFreeSize() < Size) return 0;
	_rear = _buffer + (_rear - _buffer + Size) % _buffersize;
	return Size;
}

int RingBuffer::MoveFront(int Size) noexcept
{
	if (GetUseSize() < Size) return 0;
	_front = _buffer + (_front - _buffer + Size) % _buffersize;
	return Size;
}

char* RingBuffer::GetFrontBufferPtr(void) const noexcept
{
	return _front;
}

char* RingBuffer::GetRearBufferPtr(void) const noexcept
{
	return _rear;
}
