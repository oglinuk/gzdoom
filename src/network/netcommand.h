
#pragma once

#include <memory>
#include "vectors.h"
#include "r_data/renderstyle.h"

// Maximum size of the packets sent out by the server.
#define	MAX_UDP_PACKET				8192

// This is the longest possible string we can pass over the network.
#define	MAX_NETWORK_STRING			2048

enum class NetPacketType
{
	ConnectRequest,
	ConnectResponse,
	Disconnect,
	Tic,
	SpawnActor,
	DestroyActor
};

class ByteInputStream
{
public:
	ByteInputStream() = default;
	ByteInputStream(const void *buffer, int size);

	void SetBuffer(const void *buffer, int size);

	int	ReadByte();
	int ReadShort();
	int	ReadLong();
	float ReadFloat();
	const char* ReadString();
	bool ReadBit();
	int ReadVariable();
	int ReadShortByte(int bits);
	void ReadBuffer(void* buffer, size_t length);

	bool IsAtEnd() const;
	int BytesLeft() const;

private:
	void EnsureBitSpace(int bits, bool writing);

	uint8_t *mData = nullptr; // Pointer to our stream of data
	uint8_t *pbStream; // Cursor position for next read.
	uint8_t *pbStreamEnd; // Pointer to the end of the stream. When pbStream >= pbStreamEnd, the entire stream has been read.

	uint8_t *bitBuffer = nullptr;
	int bitShift = -1;
};

class ByteOutputStream
{
public:
	ByteOutputStream() = default;
	ByteOutputStream(int size);
	ByteOutputStream(void *buffer, int size);

	void SetBuffer(int size);
	void SetBuffer(void *buffer, int size);
	void ResetPos();

	void WriteByte(int Byte);
	void WriteShort(int Short);
	void WriteLong(int Long);
	void WriteFloat(float Float);
	void WriteString(const char *pszString);
	void WriteBit(bool bit);
	void WriteVariable(int value);
	void WriteShortByte(int value, int bits);
	void WriteBuffer(const void *pvBuffer, int nLength);

	const void *GetData() const { return mData; }
	int GetSize() const { return (int)(ptrdiff_t)(pbStream - mData); }

private:
	void AdvancePointer(const int NumBytes, const bool OutboundTraffic);
	void EnsureBitSpace(int bits, bool writing);

	uint8_t *mData = nullptr; // Pointer to our stream of data
	uint8_t *pbStream; // Cursor position for next write
	uint8_t *pbStreamEnd; // Pointer to the end of the data buffer

	uint8_t *bitBuffer = nullptr;
	int bitShift = -1;

	struct DataBuffer
	{
		DataBuffer(int size) : data(new uint8_t[size]), size(size) { }
		~DataBuffer() { delete[] data; }

		uint8_t *data;
		int size;
	};
	std::shared_ptr<DataBuffer> mBuffer;
};

/**
 * \author Benjamin Berkels
 */
class NetCommand
{
	ByteOutputStream mStream;
	bool mUnreliable = false;

public:
	NetCommand ( const NetPacketType Header );

	void addInteger( const int IntValue, const int Size );
	void addByte ( const int ByteValue );
	void addShort ( const int ShortValue );
	void addLong ( const int32_t LongValue );
	void addFloat ( const float FloatValue );
	void addString ( const char *pszString );
	void addName ( FName name );
	void addBit ( const bool value );
	void addVariable ( const int value );
	void addShortByte ( int value, int bits );
	void addBuffer ( const void *pvBuffer, int nLength );
	void writeCommandToStream ( ByteOutputStream &stream ) const;
	bool isUnreliable() const { return mUnreliable; }
	void setUnreliable(bool a) { mUnreliable = a; }
	int getSize() const;
};
