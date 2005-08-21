// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2005 Andreas Huggel <ahuggel@gmx.net>
 * 
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      error.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   02-Apr-05, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "error.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const ErrMsg Error::errMsg_[] = {
        ErrMsg( -1, "Error %0: arg1=%1, arg2=%2, arg3=%3."),
        ErrMsg(  0, "Success"),
        ErrMsg(  1, "%1"), // %1=error message

        ErrMsg(  2, "%1: %2 (%3)"), // %1=path, %2=strerror, %3=function that failed
     // ErrMsg(  3, ""),

        ErrMsg(  4, "Invalid dataset name `%1'"), // %1=dataset name
        ErrMsg(  5, "Invalid record name `%1'"), // %1=record name
        ErrMsg(  6, "Invalid key `%1'"), // %1=key
        ErrMsg(  7, "Invalid tag name or ifdId `%1', ifdId %2"), // %1=tag name, %2=ifdId
        ErrMsg(  8, "Value not set"), 
        ErrMsg(  9, "%1: Failed to open the data source: %2"), // %1=path, %2=strerror
        ErrMsg( 10, "%1: Failed to open file (%2): %3"), // %1=path, %2=mode, %3=strerror
        ErrMsg( 11, "%1: The file contains data of an unknown image type"), // %1=path
        ErrMsg( 12, "The memory contains data of an unknown image type"),
        ErrMsg( 13, "Image type %1 is not supported"), // %1=image type
        ErrMsg( 14, "Failed to read image data"),
        ErrMsg( 15, "This does not look like a JPEG image"),
        ErrMsg( 16, "MakerTagInfo registry full"),
        ErrMsg( 17, "%1: Failed to rename file to %2: %3"), // %1=old path, %2=new path, %3=strerror
        ErrMsg( 18, "%1: Transfer failed: %2"), // %1=path, %2=strerror
        ErrMsg( 19, "Memory transfer failed: %1"), // %1=strerror
        ErrMsg( 20, "Failed to read input data"),
        ErrMsg( 21, "Failed to write image"),
        ErrMsg( 22, "Input data does not contain a valid image"),
        ErrMsg( 23, "Failed to create Makernote for ifdId %1"), // %1=ifdId
        ErrMsg( 24, "Entry::setValue: Value too large (tag=%1, size=%2, requested=%3)"), // %1=tag, %2=dataSize, %3=required size
        ErrMsg( 25, "Entry::setDataArea: Value too large (tag=%1, size=%2, requested=%3)"), // %1=tag, %2=dataAreaSize, %3=required size
        ErrMsg( 26, "Offset out of range"),
        ErrMsg( 27, "Unsupported data area offset type"),
        ErrMsg( 28, "Invalid charset: `%1'"), // %1=charset name
        ErrMsg( 29, "Unsupported date format"),
        ErrMsg( 30, "Unsupported time format"),

        // Last error message (message is not used)
        ErrMsg( -2, "(Unknown Error)")
    };

    int Error::errorIdx(int code)
    {
        int idx;
        for (idx = 0; errMsg_[idx].code_ != code; ++idx) {
            if (errMsg_[idx].code_ == -2) return 0;
        }
        return idx;
    }

    std::string Error::what() const
    {
        int idx = errorIdx(code_);
        std::string msg = std::string(errMsg_[idx].message_);
        std::string::size_type pos;
        pos = msg.find("%0");
        if (pos != std::string::npos) {
            msg.replace(pos, 2, toString(code_));
        }
        if (count_ > 0) {
            pos = msg.find("%1");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            pos = msg.find("%2");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
            pos = msg.find("%3");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg3_);
            }
        }
        return msg;
    }

}                                       // namespace Exiv2
