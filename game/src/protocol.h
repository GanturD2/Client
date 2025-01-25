#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

inline const char* encode_byte(char ind)
{
	static char a[8];
	*((char*)a) = ind;
	return (a);
}

inline const char* encode_2bytes(sh_int ind)
{
	static char a[8];
	*((sh_int*)a) = ind;
	return (a);
}

inline const char* encode_4bytes(int ind)
{
	static char a[8];
	*((int*)a) = ind;
	return (a);
}

inline uint8_t decode_byte(const void* a)
{
	return (*(uint8_t*)a);
}

inline uint16_t decode_2bytes(const void* a)
{
	return (*((uint16_t*)a));
}

inline INT decode_4bytes(const void* a)
{
	return (*((INT*)a));
}

#define packet_encode(buf, data, len) __packet_encode(buf, data, len, __FILE__, __LINE__)

//#define DEFAULT_PACKET_BUFFER_SIZE 32768
#define DEFAULT_PACKET_BUFFER_SIZE (150*1024)

inline bool __packet_encode(LPBUFFER pbuf, const void* data, int length, const char* file, int line)
{
	assert(nullptr != pbuf);
	assert(nullptr != data);

	if (buffer_has_space(pbuf) < length)
	{
		//sys_err("buffer length exceeded buffer size: %d, encoding %d bytes (%s:%d)", buffer_size(pbuf), length, file, line);
		return false;
	}

	//buffer_adjust_size(pbuf, length);
	buffer_write(pbuf, data, length);
	return true;
}

#endif
