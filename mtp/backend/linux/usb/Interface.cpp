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

#include <usb/Interface.h>
#include <usb/Directory.h>

namespace mtp { namespace usb
{
	Endpoint::Endpoint(const std::string &path): _maxPacketSize(0)
	{
		_addr				= Directory::ReadInt(path + "/bEndpointAddress");

		std::string	type	= Directory::ReadString(path + "/type");
		if (type == "Bulk")
			_type = EndpointType::Bulk;
		else if (type == "Control")
			_type = EndpointType::Control;
		else if (type == "Interrupt")
			_type = EndpointType::Interrupt;
		else if (type == "Isoc")
			_type = EndpointType::Isochronous;
		else
			throw std::runtime_error("invalid endpoint type " + type);

		std::string	dir		= Directory::ReadString(path + "/direction");
		if (dir == "out")
			_direction = EndpointDirection::Out;
		else if (dir == "in")
			_direction = EndpointDirection::In;
		else if (dir == "both")
			_direction = EndpointDirection::Both;
		else
			throw std::runtime_error("invalid endpoint direction " + dir);

		_maxPacketSize		= Directory::ReadInt(path + "/wMaxPacketSize");
	}

	EndpointPtr Endpoint::TryOpen(const std::string &path)
	{
		try
		{ return std::make_shared<Endpoint>(path); }
		catch(const std::exception &ex)
		{ fprintf(stderr, "failed adding endpoint: %s\n", ex.what()); }
		return nullptr;
	}

	Interface::Interface(int index, const std::string &path): _path(path)
	{
		_class		= Directory::ReadInt(path + "/bInterfaceClass");
		_subclass	= Directory::ReadInt(path + "/bInterfaceSubClass");
		_index		= Directory::ReadInt(path + "/bInterfaceNumber");

		Directory dir(path);
		while(true)
		{
			std::string entry = dir.Read();
			if (entry.empty())
				break;

			if (entry.compare(0, 3, "ep_") == 0)
			{
				EndpointPtr ep = Endpoint::TryOpen(path + "/" + entry);
				if (ep)
					_endpoints.push_back(ep);
			}
		}
	}

}}
