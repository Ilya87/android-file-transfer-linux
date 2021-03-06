/*
    This file is part of Android File Transfer For Linux.
    Copyright (C) 2015  Vladimir Menshakov

    Android File Transfer For Linux is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    Android File Transfer For Linux is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Android File Transfer For Linux.
    If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STREAM_H
#define	STREAM_H

#include <mtp/types.h>
#include <mtp/ByteArray.h>

namespace mtp
{

	class InputStream
	{
		const ByteArray &	_data;
		size_t				_offset;

	public:
		InputStream(const ByteArray & data, size_t offset = 0): _data(data), _offset(offset) { }

		const ByteArray & GetData() const
		{ return _data; }

		u8 Read8()
		{ return _data.at(_offset++); }

		u16 Read16()
		{
			u8 l = Read8();
			u8 h = Read8();
			return l | ((u16)h << 8);
		}
		u32 Read32()
		{
			u16 l = Read16();
			u16 h = Read16();
			return l | ((u32)h << 16);
		}

		u64 Read64()
		{
			u32 l = Read32();
			u32 h = Read32();
			return l | ((u64)h << 32);
		}

		std::string ReadString()
		{ return ReadString(Read8()); }

		std::string ReadString(unsigned len)
		{
			std::string str;
			str.reserve(2 * len);
			while(len--)
			{
				u16 ch = Read16();
				if (ch == 0)
					continue;
				if (ch <= 0x7f)
					str += (char)ch;
				else if (ch <= 0x7ff)
				{
					str += (char) ((ch >> 6) | 0xc0);
					str += (char) ((ch & 0x3f) | 0x80);
				}
				else //if (ch <= 0xffff)
				{
					str += (char)((ch >> 12) | 0xe0);
					str += (char)(((ch & 0x0fc0) >> 6) | 0x80);
					str += (char)( (ch & 0x003f) | 0x80);
				}
			}
			return str;
		}

		template<typename ElementType>
		std::vector<ElementType> ReadArray()
		{
			std::vector<ElementType> array;
			u32 size = Read32();
			while(size--)
			{
				ElementType el;
				(*this) >> el;
				array.push_back(el);
			}
			return array;
		}
	};

	inline InputStream & operator >> (InputStream &stream, u8 &value)
	{ value = stream.Read8(); return stream; }

	inline InputStream & operator >> (InputStream &stream, u16 &value)
	{ value = stream.Read16(); return stream; }

	inline InputStream & operator >> (InputStream &stream, u32 &value)
	{ value = stream.Read32(); return stream; }

	inline InputStream & operator >> (InputStream &stream, u64 &value)
	{ value = stream.Read64(); return stream; }

	inline InputStream & operator >> (InputStream &stream, std::string &value)
	{ value = stream.ReadString(); return stream; }

	template<typename ElementType>
	inline InputStream & operator >> (InputStream &stream, std::vector<ElementType> &value)
	{ value = stream.template ReadArray<ElementType>(); return stream; }

}


#endif	/* STREAM_H */
