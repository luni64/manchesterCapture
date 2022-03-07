#pragma once

#include <Arduino.h>

struct RingBuffer
{
    uint32_t buff[256]; // needs to be 256 for simple rollover (head/tail are 8bit variables)
    byte head = 0;
    byte tail = 0;

    void push(uint32_t value)
    {
        buff[head++] = value;
    }

    uint32_t pop()
    {
        return buff[tail++];
    }

    bool hasElements() { return head != tail; }
};

template <typename ET, size_t S>
class RingBuf
{
 public:
    /* Constructor. Init mReadIndex to 0 and mSize to 0 */
    RingBuf();

    /* Push a data at the end of the buffer */
    bool push(const ET inElement);

    /* Pop the data at the beginning of the buffer */
    bool pop(ET& outElement);

    /* Return true if the buffer is full */
    bool isFull() const { return mSize == S; }

    /* Return true if the buffer is empty */
    bool isEmpty() const { return mSize == 0; }

    /* Reset the buffer  to an empty state */
    void clear() { mSize = 0; }

    /* return the size of the buffer */
    size_t size() const { return mSize; }

    /* return the maximum size of the buffer */
    size_t maxSize() const { return S; }

 private:
    ET mBuffer[S];
    size_t mReadIndex;
    size_t mSize;

    size_t writeIndex();
};

template <typename ET, size_t S>
size_t RingBuf<ET, S>::writeIndex()
{
    size_t wi = mReadIndex + mSize;
    if (wi >= S) wi -= S;
    return wi;
}

template <typename ET, size_t S>
RingBuf<ET, S>::RingBuf()
    : mReadIndex(0), mSize(0)
{
}

template <typename ET, size_t S>
bool RingBuf<ET, S>::push(const ET inElement)
{
    if (isFull()) return false;
    mBuffer[writeIndex()] = inElement;
    mSize++;
    return true;
}

template <typename ET, size_t S>
bool RingBuf<ET, S>::pop(ET& outElement)
{
    if (isEmpty()) return false;
    outElement = mBuffer[mReadIndex];
    mReadIndex++;
    mSize--;
    if (mReadIndex == S) mReadIndex = 0;
    return true;
}