// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
 *
 * Lens database to decode Exif.Nikon3.LensData
 * Copyright (C) 2005, 2006 Robert Rottmerhusen <email@rottmerhusen.com>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      nikonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   17-May-04, ahu: created
             25-May-04, ahu: combined all Nikon formats in one component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "nikonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "image.hpp"
#include "tags.hpp"
#include "error.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstring>

#define EXV_HAVE_LENSDATA

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! Focus area for Nikon cameras.
    static const char *nikonFocusarea[] = {
        N_("Single area"),
        N_("Dynamic area"),
        N_("Dynamic area, closest subject"),
        N_("Group dynamic"),
        N_("Single area (wide)"),
        N_("Dynamic area (wide")
    };

    // Roger Larsson: My guess is that focuspoints will follow autofocus sensor
    // module. Note that relative size and position will vary depending on if
    // "wide" or not
    //! Focus points for Nikon cameras, used for Nikon 1 and Nikon 3 makernotes.
    static const char *nikonFocuspoints[] = {
        N_("Center"),
        N_("Top"),
        N_("Bottom"),
        N_("Left"),
        N_("Right"),
        N_("Upper-left"),
        N_("Upper-right"),
        N_("Lower-left"),
        N_("Lower-right"),
        N_("Left-most"),
        N_("Right-most")
    };

    //! ColorSpace, tag 0x001e
    extern const TagDetails nikonColorSpace[] = {
        { 1, N_("sRGB")      },
        { 2, N_("Adobe RGB") }
    };

    //! FlashMode, tag 0x0087
    extern const TagDetails nikonFlashMode[] = {
        { 0, N_("Did not fire")         },
        { 1, N_("Fire, manual")         },
        { 7, N_("Fire, external")       },
        { 8, N_("Fire, commander mode") },
        { 9, N_("Fire, TTL mode")       }
    };

    //! ShootingMode, tag 0x0089
    extern const TagDetails nikonShootingMode[] = {
        {  1, N_("Continuous")               },
        {  2, N_("Delay")                    },
        {  4, N_("PC control")               },
        {  8, N_("Exposure bracketing")      },
        { 16, N_("Unused LE-NR slowdown")    },
        { 32, N_("White balance bracketing") },
        { 64, N_("IR control")               }
    };

    //! AutoBracketRelease, tag 0x008a
    extern const TagDetails nikonAutoBracketRelease[] = {
        { 0, N_("None")           },
        { 1, N_("Auto release")   },
        { 2, N_("Manual release") }
    };

    //! HighISONoiseReduction, tag 0x00b1
    extern const TagDetails nikonHighISONoiseReduction[] = {
        { 0, N_("Off")                  },
        { 1, N_("On for ISO 1600/3200") },
        { 2, N_("Weak")                 },
        { 4, N_("Normal")               },
        { 6, N_("Strong")               }
    };

    //! @cond IGNORE
    Nikon1MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("NIKON*", "*", createNikonMakerNote);
        MakerNoteFactory::registerMakerNote(
            nikon1IfdId, MakerNote::AutoPtr(new Nikon1MakerNote));

        ExifTags::registerMakerTagInfo(nikon1IfdId, tagInfo_);
    }
    //! @endcond

    // Nikon1 MakerNote Tag Info
    const TagInfo Nikon1MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", N_("Version"), 
                N_("Nikon Makernote version"), 
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", N_("ISO Speed"),
                N_("ISO speed setting"), 
                nikon1IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", N_("Color Mode"), 
                N_("Color mode"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", N_("Quality"), 
                N_("Image quality setting"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", N_("White Balance"), 
                N_("White balance"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", N_("Sharpening"), 
                N_("Image sharpening setting"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", N_("Focus"), 
                N_("Focus mode"), 
                nikon1IfdId, makerTags, asciiString, print0x0007),
        TagInfo(0x0008, "FlashSetting", N_("Flash Setting"), 
                N_("Flash setting"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0009, "FlashDevice", N_("Flash Device"), 
                N_("Flash device"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "0x000a", "0x000a", 
                N_("Unknown"), 
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000b, "WBFineTune", N_("WB Fine Tune"),
                N_("White balance fine tune"), 
                nikon1IfdId, makerTags, signedShort, printValue),
        TagInfo(0x000c, "ColorBalance1", N_("Color Balance 1"),
                N_("Color balance 1"), 
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x000d, "ProgramShift", N_("Program Shift"),
                N_("Program shift"), 
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x000e, "ExposureDiff", N_("Exposure Difference"),
                N_("Exposure difference"), 
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x000f, "ISOSelection", N_("ISO Selection"), 
                N_("ISO selection"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", N_("Data Dump"), 
                N_("Data dump"), 
                nikon1IfdId, makerTags, undefined, printValue),

        // TODO: Add Preview decoding implementation.
        TagInfo(0x0011, "Preview", N_("Preview Informations"), 
                N_("Preview informations"), 
                minoltaIfdId, makerTags, undefined, printValue),

        TagInfo(0x0012, "FlashBias", N_("Flash Bias"), 
                N_("Flash exposure compensation"), 
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0013, "ISOSettings", N_("ISO Settings"),
                N_("ISO setting"), 
                nikon1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "ImageBoundary", N_("Image Boundary"),
                N_("Image boundary"), 
                nikon1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0018, "FlashBiasBracket", N_("Flash Bias Bracket"), 
                N_("Flash exposure bracket value"), 
                minoltaIfdId, makerTags, undefined, printValue),
        TagInfo(0x0019, "ExposureBracket", N_("Exposure Bracket"),
                N_("Exposure bracket value"), 
                nikon1IfdId, makerTags, signedRational, printValue),
        TagInfo(0x001a, "ImageProcessing", N_("Image Processing"), 
                N_("Image processing"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x001b, "CropHiSpeed", N_("Crop High Speed"),
                N_("Crop high speed"), 
                nikon1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001d, "SerialNumber", N_("Serial Number"), 
                N_("Serial Number"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x001e, "ColorSpace", N_("Color Space"),
                N_("Color space"), 
                nikon1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonColorSpace)),
        TagInfo(0x0080, "ImageAdjustment", N_("Image Adjustment"), 
                N_("Image adjustment setting"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0081, "ToneComp", N_("Tone Compensation"), 
                N_("Tone compensation"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "Adapter", N_("Lens Adapter"), 
                N_("Lens adapter used"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0083, "LensType", N_("Lens Type"),
                N_("Lens type"), 
                nikon1IfdId, makerTags, unsignedByte, print0x0083),
        TagInfo(0x0084, "Lens", N_("Lens"),
                N_("Lens"), 
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0085, "FocusDistance", N_("Focus Distance"), 
                N_("Manual focus distance"), 
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0086, "DigitalZoom", N_("Digital Zoom"), 
                N_("Digital zoom setting"), 
                nikon1IfdId, makerTags, unsignedRational, print0x0086),
        TagInfo(0x0087, "FlashMode", N_("Flash Mode"),
                N_("Flash mode"), 
                nikon1IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikonFlashMode)),
        TagInfo(0x0088, "AFFocusPos", N_("AF Focus Position"), 
                N_("AF focus position information"), 
                nikon1IfdId, makerTags, undefined, print0x0088),
        TagInfo(0x0089, "ShootingMode", N_("Shooting Mode"),
                N_("Shooting mode"), 
                nikon1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonShootingMode)),
        TagInfo(0x008a, "AutoBracketRelease", N_("Auto Bracket Release"),
                N_("Auto bracket release"), 
                nikon1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonAutoBracketRelease)),
        TagInfo(0x008b, "LensFStops", N_("Lens FStops"),
                N_("Lens FStops"), 
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x008c, "NEFCurve1", N_("NEF Curve 1"),
                N_("NEF curve 1"), 
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x008d, "ColorHue", N_("Color Hue"), 
                N_("Color hue"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x008f, "SceneMode", N_("Scene Mode"), 
                N_("Scene mode"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0090, "LightSource", N_("Light Source"), 
                N_("Light source"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0092, "HueAdjustment", N_("Hue Adjustment"),
                N_("Hue adjustment"), 
                nikon1IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0094, "Saturation", N_("Saturation"),
                N_("Saturation"), 
                nikon1IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0095, "NoiseReduction", N_("Noise Reduction"), 
                N_("Noise reduction"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0096, "NEFCurve2", N_("NEF Curve 2"),
                N_("NEF curve 2"), 
                nikon1IfdId, makerTags, undefined, printValue),

        // TODO: Add Color Balance settings decoding implementation.
        TagInfo(0x0097, "ColorBalance", N_("Color Balance"),
                N_("Color balance settings"), 
                nikon1IfdId, makerTags, undefined, printValue),

        // TODO: Add Lens Data decoding implementation.
        TagInfo(0x0098, "LensData", N_("Lens Data"),
                N_("Lens data settings"), 
                nikon1IfdId, makerTags, undefined, printValue),

        TagInfo(0x0099, "RawImageCenter", N_("Raw Image Center"),
                N_("Raw image center"), 
                nikon1IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009a, "SensorPixelSize", N_("Sensor Pixel Size"), 
                N_("Sensor pixel size"), 
                nikon1IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x00a0, "SerialNumber", N_("Serial Number"), 
                N_("Serial number"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a2, "ImageDataSize", N_("Image Data Size"), 
                N_("Image data size"), 
                nikon1IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a5, "ImageCount", N_("Image Count"), 
                N_("Image count"), 
                nikon1IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a6, "DeleteImageCount", N_("Delete Image Count"), 
                N_("Delete image count"), 
                nikon1IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a7, "ShutterCount", N_("Shutter Count"),
                N_("Shutter Count"), 
                nikon1IfdId, makerTags, undefined, printValue),
        TagInfo(0x00a9, "ImageOptimization", N_("Image Optimization"), 
                N_("Image optimization"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00aa, "Saturation", N_("Saturation"), 
                N_("Saturation"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ab, "VariProgram", N_("Program Variation"), 
                N_("Program variation"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ac, "ImageStabilization", N_("Image Stabilization"), 
                N_("Image stabilization"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ad, "AFResponse", N_("AF Response"), 
                N_("AF response"), 
                nikon1IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00b1, "HighISONoiseReduction", N_("High ISO Noise Reduction"),
                N_("High ISO Noise Reduction"), 
                nikon1IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikonHighISONoiseReduction)),
        TagInfo(0x0e00, "PrintIM", N_("Print IM"), 
                N_("PrintIM information"), 
                nikon1IfdId, makerTags, undefined, printValue),

        // TODO: Add Capture Data decoding implementation.
        TagInfo(0x0e01, "CaptureData", N_("Capture Data"), 
                N_("Capture data"), 
                nikon1IfdId, makerTags, undefined, printValue),

        TagInfo(0x0e09, "CaptureVersion", N_("Capture Version"), 
                N_("Capture version"), 
                nikon1IfdId, makerTags, asciiString, printValue),

        // TODO: Add Capture Offsets decoding implementation.
        TagInfo(0x0e0e, "CaptureOffsets", N_("Capture Offsets"), 
                N_("Capture offsets"), 
                nikon1IfdId, makerTags, undefined, printValue),

        // End of list marker
        TagInfo(0xffff, "(UnknownNikon1MnTag)", "(UnknownNikon1MnTag)", 
                N_("Unknown Nikon1MakerNote tag"), 
                nikon1IfdId, makerTags, invalidTypeId, printValue)
    };

    Nikon1MakerNote::Nikon1MakerNote(bool alloc)
        : IfdMakerNote(nikon1IfdId, alloc)
    {
    }

    Nikon1MakerNote::Nikon1MakerNote(const Nikon1MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon1MakerNote* Nikon1MakerNote::create_(bool alloc) const
    {
        return new Nikon1MakerNote(alloc);
    }

    Nikon1MakerNote::AutoPtr Nikon1MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon1MakerNote* Nikon1MakerNote::clone_() const
    {
        return new Nikon1MakerNote(*this);
    }

    std::ostream& Nikon1MakerNote::print0x0002(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0007(std::ostream& os,
                                               const Value& value)
    {
        std::string focus = value.toString();
        if      (focus == "AF-C  ") os << _("Continuous autofocus");
        else if (focus == "AF-S  ") os << _("Single autofocus");
        else                      os << "(" << value << ")";
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0083(std::ostream& os,
                                               const Value& value)
    {
        unsigned char lensType = (unsigned char)value.toLong();

        if (lensType & 0x01) os << "MF ";
        if (lensType & 0x02) os << "D ";
        if (lensType & 0x04) os << "G ";
        if (lensType & 0x08) os << "VR";
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0085(std::ostream& os,
                                               const Value& value)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << _("Unknown");
        }
        else if (distance.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(2)
               << (float)distance.first / distance.second
               << " m";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0086(std::ostream& os,
                                               const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << _("Not used");
        }
        else if (zoom.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second
               << "x";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon1MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() >= 1) {
            unsigned long focusArea = value.toLong(0);
            os << nikonFocusarea[focusArea] ;
        }
        if (value.count() >= 2) {
            os << "; ";
            unsigned long focusPoint = value.toLong(1);

            switch (focusPoint) {
            // Could use array nikonFokuspoints
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                os << nikonFocuspoints[focusPoint];
                break;
            default:
                os << value;
                if (focusPoint < sizeof(nikonFocuspoints)/sizeof(nikonFocuspoints[0]))
                    os << " " << _("guess") << " " << nikonFocuspoints[focusPoint];
                break;
            }
        }
        if (value.count() >= 3) {
            unsigned long focusPointsUsed1 = value.toLong(2);
            unsigned long focusPointsUsed2 = value.toLong(3);

            if (focusPointsUsed1 != 0 && focusPointsUsed2 != 0)
            {
                os << "; [";
    
                if (focusPointsUsed1 & 1)
                    os << nikonFocuspoints[0] << " ";
                if (focusPointsUsed1 & 2)
                    os << nikonFocuspoints[1] << " ";
                if (focusPointsUsed1 & 4)
                    os << nikonFocuspoints[2] << " ";
                if (focusPointsUsed1 & 8)
                    os << nikonFocuspoints[3] << " ";
                if (focusPointsUsed1 & 16)
                    os << nikonFocuspoints[4] << " ";
                if (focusPointsUsed1 & 32)
                    os << nikonFocuspoints[5] << " ";
                if (focusPointsUsed1 & 64)
                    os << nikonFocuspoints[6] << " ";
                if (focusPointsUsed1 & 128)
                    os << nikonFocuspoints[7] << " ";
    
                if (focusPointsUsed2 & 1)
                    os << nikonFocuspoints[8] << " ";
                if (focusPointsUsed2 & 2)
                    os << nikonFocuspoints[9] << " ";
                if (focusPointsUsed2 & 4)
                    os << nikonFocuspoints[10] << " ";

                os << "]";
            }
        }
        else {
            os << value;
        }
        return os;
    }

    //! @cond IGNORE
    Nikon2MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            nikon2IfdId, MakerNote::AutoPtr(new Nikon2MakerNote));

        ExifTags::registerMakerTagInfo(nikon2IfdId, tagInfo_);
    }
    //! @endcond

    //! Quality, tag 0x0003
    extern const TagDetails nikon2Quality[] = {
        { 1, "VGA Basic"   },
        { 2, "VGA Normal"  },
        { 3, "VGA Fine"    },
        { 4, "SXGA Basic"  },
        { 5, "SXGA Normal" },
        { 6, "SXGA Fine"   }
    };

    //! ColorMode, tag 0x0004
    extern const TagDetails nikon2ColorMode[] = {
        { 1, "Color"      },
        { 2, "Monochrome" }
    };

    //! ImageAdjustment, tag 0x0005
    extern const TagDetails nikon2ImageAdjustment[] = {
        { 0, "Normal"    },
        { 1, "Bright+"   },
        { 2, "Bright-"   },
        { 3, "Contrast+" },
        { 4, "Contrast-" }
    };

    //! ISOSpeed, tag 0x0006
    extern const TagDetails nikon2IsoSpeed[] = {
        { 0, "80"  },
        { 2, "160" },
        { 4, "320" },
        { 5, "100" }
    };

    //! WhiteBalance, tag 0x0007
    extern const TagDetails nikon2WhiteBalance[] = {
        { 0, "Auto"         },
        { 1, "Preset"       },
        { 2, "Daylight"     },
        { 3, "Incandescent" },
        { 4, "Fluorescent"  },
        { 5, "Cloudy"       },
        { 6, "Speedlight"   }
    };

    // Nikon2 MakerNote Tag Info
    const TagInfo Nikon2MakerNote::tagInfo_[] = {
        TagInfo(0x0002, "0x0002", "0x0002", "Unknown", nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0003, "Quality", "Quality", "Image quality setting", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2Quality)),
        TagInfo(0x0004, "ColorMode", "ColorMode", "Color mode", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ColorMode)),
        TagInfo(0x0005, "ImageAdjustment", "ImageAdjustment", "Image adjustment setting", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2ImageAdjustment)),
        TagInfo(0x0006, "ISOSpeed", "ISOSpeed", "ISO speed setting", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2IsoSpeed)),
        TagInfo(0x0007, "WhiteBalance", "WhiteBalance", "White balance", nikon2IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon2WhiteBalance)),
        TagInfo(0x0008, "Focus", "Focus", "Focus mode", nikon2IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", "Unknown", nikon2IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "DigitalZoom", "DigitalZoom", "Digital zoom setting", nikon2IfdId, makerTags, unsignedRational, print0x000a),
        TagInfo(0x000b, "Adapter", "Adapter", "Adapter used", nikon2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0f00, "0x0f00", "0x0f00", "Unknown", nikon2IfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon2MnTag)", "(UnknownNikon2MnTag)", "Unknown Nikon2MakerNote tag", nikon2IfdId, makerTags, invalidTypeId, printValue)
    };

    Nikon2MakerNote::Nikon2MakerNote(bool alloc)
        : IfdMakerNote(nikon2IfdId, alloc)
    {
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0', 0x00, 0x01
        };
        readHeader(buf, 8, byteOrder_);
    }

    Nikon2MakerNote::Nikon2MakerNote(const Nikon2MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int Nikon2MakerNote::readHeader(const byte* buf,
                                    long        len,
                                    ByteOrder   /*byteOrder*/)
    {
        if (len < 8) return 1;

        header_.alloc(8);
        memcpy(header_.pData_, buf, header_.size_);
        start_ = 8;
        return 0;
    }

    int Nikon2MakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Nikon prefix
        if (   header_.size_ < 8
            || std::string(reinterpret_cast<char*>(header_.pData_), 6)
                    != std::string("Nikon\0", 6)) {
            rc = 2;
        }
        return rc;
    }

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon2MakerNote* Nikon2MakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new Nikon2MakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    Nikon2MakerNote::AutoPtr Nikon2MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon2MakerNote* Nikon2MakerNote::clone_() const
    {
        return new Nikon2MakerNote(*this);
    }

    std::ostream& Nikon2MakerNote::print0x000a(std::ostream& os,
                                               const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.first == 0) {
            os << "Not used";
        }
        else if (zoom.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second
               << "x";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    //! @cond IGNORE
    Nikon3MakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote(
            nikon3IfdId, MakerNote::AutoPtr(new Nikon3MakerNote));

        ExifTags::registerMakerTagInfo(nikon3IfdId, tagInfo_);
    }
    //! @endcond

    //! FlashComp, tag 0x0012
    extern const TagDetails nikon3FlashComp[] = {
        // From the PHP JPEG Metadata Toolkit
        { 0x06, "+1.0 EV" },
        { 0x04, "+0.7 EV" },
        { 0x03, "+0.5 EV" },
        { 0x02, "+0.3 EV" },
        { 0x00,  "0.0 EV" },
        { 0xfe, "-0.3 EV" },
        { 0xfd, "-0.5 EV" },
        { 0xfc, "-0.7 EV" },
        { 0xfa, "-1.0 EV" },
        { 0xf8, "-1.3 EV" },
        { 0xf7, "-1.5 EV" },
        { 0xf6, "-1.7 EV" },
        { 0xf4, "-2.0 EV" },
        { 0xf2, "-2.3 EV" },
        { 0xf1, "-2.5 EV" },
        { 0xf0, "-2.7 EV" },
        { 0xee, "-3.0 EV" }
    };

    //! FlashType, tag 0x0087
    extern const TagDetails nikon3FlashType[] = {
        // From Exiftool
        { 0, "Not used"              },
        { 8, "Fired, commander mode" },
        { 9, "Fired, TTL mode"       }
    };

    //! Bracketing, tag 0x0089
    extern const TagDetails nikon3Bracketing[] = {
        // From Exiftool
        { 0x00, "Single"                   },
        { 0x01, "Continuous"               },
        { 0x02, "Delay"                    },
        { 0x03, "Remote with delay"        },
        { 0x04, "Remote"                   },
        { 0x16, "Exposure bracketing"      },
        { 0x64, "White balance bracketing" }
    };

    // Nikon3 MakerNote Tag Info
    const TagInfo Nikon3MakerNote::tagInfo_[] = {
        TagInfo(0x0001, "Version", "Version", "Nikon Makernote version", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISOSpeed", "ISO speed used", nikon3IfdId, makerTags, unsignedShort, print0x0002),
        TagInfo(0x0003, "ColorMode", "ColorMode", "Color mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0004, "Quality", "Quality", "Image quality setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0005, "WhiteBalance", "WhiteBalance", "White balance", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0006, "Sharpening", "Sharpening", "Image sharpening setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "Focus", "Focus", "Focus mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "FlashSetting", "FlashSetting", "Flash setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0009, "FlashMode", "FlashMode", "Flash mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x000b, "WhiteBalanceBias", "WhiteBalanceBias", "White balance bias", nikon3IfdId, makerTags, signedShort, printValue),
//        TagInfo(0x000c, "ColorBalance1", "ColorBalance1", "Color balance 1", nikon3IfdId, makerTags, xxx, printValue),
        TagInfo(0x000d, "0x000d", "0x000d", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000e, "ExposureDiff", "ExposureDiff", "Exposure difference", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x000f, "ISOSelection", "ISOSelection", "ISO selection", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0010, "DataDump", "DataDump", "Data dump", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0011, "ThumbOffset", "ThumbOffset", "Thumbnail IFD offset", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0012, "FlashComp", "FlashComp", "Flash compensation setting", nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikon3FlashComp)),
        TagInfo(0x0013, "ISOSetting", "ISOSetting", "ISO speed setting", nikon3IfdId, makerTags, unsignedShort, print0x0002), // use 0x0002 print fct
        TagInfo(0x0016, "ImageBoundry", "ImageBoundry", "Image boundry", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0017, "0x0017", "0x0017", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0018, "FlashBracketComp", "FlashBracketComp", "Flash bracket compensation applied", nikon3IfdId, makerTags, undefined, EXV_PRINT_TAG(nikon3FlashComp)), // use 0x0012 print fct
        TagInfo(0x0019, "ExposureBracketComp", "ExposureBracketComp", "AE bracket compensation applied", nikon3IfdId, makerTags, signedRational, printValue),
        TagInfo(0x0080, "ImageAdjustment", "ImageAdjustment", "Image adjustment setting", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0081, "ToneComp", "ToneComp", "Tone compensation setting (contrast)", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0082, "AuxiliaryLens", "AuxiliaryLens", "Auxiliary lens (adapter)", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0083, "LensType", "LensType", "Lens type", nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x0084, "Lens", "Lens", "Lens", nikon3IfdId, makerTags, unsignedRational, print0x0084),
        TagInfo(0x0085, "FocusDistance", "FocusDistance", "Manual focus distance", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0086, "DigitalZoom", "DigitalZoom", "Digital zoom setting", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x0087, "FlashType", "FlashType", "Type of flash used", nikon3IfdId, makerTags, unsignedByte, EXV_PRINT_TAG(nikon3FlashType)),
        TagInfo(0x0088, "AFFocusPos", "AFFocusPos", "AF focus position", nikon3IfdId, makerTags, undefined, print0x0088),
        TagInfo(0x0089, "Bracketing", "Bracketing", "Bracketing", nikon3IfdId, makerTags, unsignedShort, EXV_PRINT_TAG(nikon3Bracketing)),
        TagInfo(0x008a, "0x008a", "0x008a", "Unknown", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x008b, "LensFStops", "LensFStops", "Number of lens stops", nikon3IfdId, makerTags, undefined, print0x008b),
        TagInfo(0x008c, "ToneCurve", "ToneCurve", "Tone curve", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x008d, "ColorMode", "ColorMode", "Color mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x008f, "SceneMode", "SceneMode", "Scene mode", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0090, "LightingType", "LightingType", "Lighting type", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0091, "0x0091", "0x0091", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0092, "HueAdjustment", "HueAdjustment", "Hue adjustment", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0094, "Saturation", "Saturation", "Saturation adjustment", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x0095, "NoiseReduction", "NoiseReduction", "Noise reduction", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x0096, "CompressionCurve", "CompressionCurve", "Compression curve", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0097, "ColorBalance2", "ColorBalance2", "Color balance 2", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x0098, "LensData", "LensData", "Lens data", nikon3IfdId, makerTags, undefined, print0x0098),
        TagInfo(0x0099, "NEFThumbnailSize", "NEFThumbnailSize", "NEF thumbnail size", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009a, "SensorPixelSize", "SensorPixelSize", "Sensor pixel size", nikon3IfdId, makerTags, unsignedRational, printValue),
        TagInfo(0x009b, "0x009b", "0x009b", "Unknown", nikon3IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x009f, "0x009f", "0x009f", "Unknown", nikon3IfdId, makerTags, signedShort, printValue),
        TagInfo(0x00a0, "SerialNumber", "SerialNumber", "Camera serial number", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a2, "0x00a2", "0x00a2", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a3, "0x00a3", "0x00a3", "Unknown", nikon3IfdId, makerTags, unsignedByte, printValue),
        TagInfo(0x00a5, "0x00a5", "0x00a5", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a6, "0x00a6", "0x00a6", "Unknown", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a7, "ShutterCount", "ShutterCount", "Number of shots taken by camera", nikon3IfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x00a8, "0x00a8", "0x00a8", "Unknown", nikon3IfdId, makerTags, undefined, printValue),
        TagInfo(0x00a9, "ImageOptimization", "ImageOptimization", "Image optimization", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00aa, "Saturation", "Saturation", "Saturation", nikon3IfdId, makerTags, asciiString, printValue),
        TagInfo(0x00ab, "VariProgram", "VariProgram", "Vari program", nikon3IfdId, makerTags, asciiString, printValue),
//        TagInfo(0x0e00, "PrintIM", "PrintIM", "Print image matching", nikon3IfdId, makerTags, xxx, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownNikon3MnTag)", "(UnknownNikon3MnTag)", "Unknown Nikon3MakerNote tag", nikon3IfdId, makerTags, invalidTypeId, printValue)
    };

    Nikon3MakerNote::Nikon3MakerNote(bool alloc)
        : IfdMakerNote(nikon3IfdId, alloc)
    {
        absShift_ = false;
        byte buf[] = {
            'N', 'i', 'k', 'o', 'n', '\0',
            0x02, 0x10, 0x00, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08
        };
        readHeader(buf, 18, byteOrder_);
    }

    Nikon3MakerNote::Nikon3MakerNote(const Nikon3MakerNote& rhs)
        : IfdMakerNote(rhs)
    {
    }

    int Nikon3MakerNote::readHeader(const byte* buf,
                                    long        len,
                                    ByteOrder   /*byteOrder*/)
    {
        if (len < 18) return 1;

        header_.alloc(18);
        memcpy(header_.pData_, buf, header_.size_);
        TiffHeader tiffHeader;
        tiffHeader.read(header_.pData_ + 10);
        byteOrder_ = tiffHeader.byteOrder();
        start_ = 10 + tiffHeader.offset();
        shift_ = 10;
        return 0;
    }

    int Nikon3MakerNote::checkHeader() const
    {
        int rc = 0;
        // Check the Nikon prefix
        if (   header_.size_ < 18
            || std::string(reinterpret_cast<char*>(header_.pData_), 6)
                    != std::string("Nikon\0", 6)) {
            rc = 2;
        }
        return rc;
    }

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    Nikon3MakerNote* Nikon3MakerNote::create_(bool alloc) const
    {
        AutoPtr makerNote(new Nikon3MakerNote(alloc));
        assert(makerNote.get() != 0);
        makerNote->readHeader(header_.pData_, header_.size_, byteOrder_);
        return makerNote.release();
    }

    Nikon3MakerNote::AutoPtr Nikon3MakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    Nikon3MakerNote* Nikon3MakerNote::clone_() const
    {
        return new Nikon3MakerNote(*this);
    }

    std::ostream& Nikon3MakerNote::print0x0002(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() > 1) {
            os << value.toLong(1);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0084(std::ostream& os,
                                               const Value& value)
    {
        if (value.count() == 4) {
            long len1 = value.toLong(0);
            long len2 = value.toLong(1);
            Rational fno1 = value.toRational(2);
            Rational fno2 = value.toRational(3);
            os << len1;
            if (len2 != len1) {
                os << "-" << len2;
            }
            os << "mm ";
            std::ostringstream oss;
            oss.copyfmt(os);
            os << "F" << std::setprecision(2)
               << static_cast<float>(fno1.first) / fno1.second;
            if (fno2 != fno1) {
                os << "-" << std::setprecision(2)
                   << static_cast<float>(fno2.first) / fno2.second;
            }
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& Nikon3MakerNote::print0x0088(std::ostream& os,
                                               const Value& value)
    {
        if (value.size() != 4) { // Size is 4 even for those who map this way...
            os << "(" << value << ")";
        }
        else {
            // Mapping by Roger Larsson
            unsigned focusmetering = value.toLong(0);
            unsigned focuspoint = value.toLong(1);
            unsigned focusused = (value.toLong(2) << 8) + value.toLong(3);
            enum {standard, wide} combination = standard;
            const unsigned focuspoints =   sizeof(nikonFocuspoints)
                                         / sizeof(nikonFocuspoints[0]);

            if (focusmetering == 0 && focuspoint == 0 && focusused == 0) {
                // Special case, in Manual focus and with Nikon compacts
                // this indicates that the field has no meaning.
                // But when acually in "Single area, Center" this can mean
                // that focus was not found (try this in AF-C mode)
                // TODO: handle the meaningful case (interacts with other fields)
                os << "N/A";
                return os;
            }

            switch (focusmetering) {
            case 0x00: os << "Single area"; break; // D70, D200
            case 0x01: os << "Dynamic area"; break; // D70, D200
            case 0x02: os << "Closest subject"; break; // D70, D200
            case 0x03: os << "Group dynamic-AF"; break; // D200
            case 0x04: os << "Single area (wide)"; combination = wide; break; // D200
            case 0x05: os << "Dynamic area (wide)"; combination = wide; break; // D200
            default: os << "(" << focusmetering << ")"; break;
            }

            char sep = ';';
            if (focusmetering != 0x02) { //  No user selected point for Closest subject
                os << sep << ' ';

                // What focuspoint did the user select?
                if (focuspoint < focuspoints) {
                    os << nikonFocuspoints[focuspoint];
                    // TODO: os << position[fokuspoint][combination]
                }
                else
                    os << "(" << focuspoint << ")";

                sep = ',';
            }

            // What fokuspoints(!) did the camera use? add if differs
            if (focusused == 0)
                os << sep << " none";
            else if (focusused != 1U<<focuspoint) {
                // selected point was not the actually used one
                // (Roger Larsson: my interpretation, verify)
                os << sep;
                for (unsigned fpid=0; fpid<focuspoints; fpid++)
                    if (focusused & 1<<fpid)
                        os << ' ' << nikonFocuspoints[fpid];
            }

            os << " used";
        }

        return os;
    }

    std::ostream& Nikon3MakerNote::print0x008b(std::ostream& os,
                                               const Value& value)
    {
        // Decoded by Robert Rottmerhusen <email@rottmerhusen.com>
        if (value.size() != 4) return os << "(" << value << ")";
        float a = value.toFloat(0);
        long  b = value.toLong(1);
        long  c = value.toLong(2);
        if (c == 0) return os << "(" << value << ")";
        return os << a * b / c;
    }

    std::ostream& Nikon3MakerNote::print0x0098(std::ostream& os,
                                               const Value& value)
    {
#ifdef EXV_HAVE_LENSDATA
        //#-----------------------------------------
        //# List of AF F-Mount lenses - version 2.02
        //#-----------------------------------------
        //#
        //# created by Robert Rottmerhusen 2005 - 2006
        //# http://www.rottmerhusen.com (info@rottmerhusen.com)
        //#
        //# with great help of Hiroshi Kamisaka (many new lenses)
        //# http://homepage3.nifty.com/kamisaka/
        //#
        //# for use in non-commercial, GPL or open source software only!
        //# please contact me for adding lenses or use in commercial software.
        //#
        //#"data from TAG 0x98" "ltyp""manuf" "lens name from manuf";
        //#
        struct {unsigned char lid,stps,focs,focl,aps,apl,lfw, ltype; char *manuf, *lensname;}
        fmountlens[] = {
            {0x01,0x58,0x50,0x50,0x14,0x14,0x02,0x00, "Nikon", "AF Nikkor 50mm f/1.8"},
            {0x02,0x42,0x44,0x5C,0x2A,0x34,0x02,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
            {0x02,0x42,0x44,0x5C,0x2A,0x34,0x08,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5"},
            {0x03,0x48,0x5C,0x81,0x30,0x30,0x02,0x00, "Nikon", "AF Zoom-Nikkor 70-210mm f/4"},
            {0x04,0x48,0x3C,0x3C,0x24,0x24,0x03,0x00, "Nikon", "AF Nikkor 28mm f/2.8"},
            {0x05,0x54,0x50,0x50,0x0C,0x0C,0x04,0x00, "Nikon", "AF Nikkor 50mm f/1.4"},
            {0x06,0x54,0x53,0x53,0x24,0x24,0x06,0x00, "Nikon", "AF Micro-Nikkor 55mm f/2.8"},
            {0x07,0x40,0x3C,0x62,0x2C,0x34,0x03,0x00, "Nikon", "AF Zoom-Nikkor 28-85mm f/3.5-4.5"},
            {0x08,0x40,0x44,0x6A,0x2C,0x34,0x04,0x00, "Nikon", "AF Zoom-Nikkor 35-105mm f/3.5-4.5"},
            {0x09,0x48,0x37,0x37,0x24,0x24,0x04,0x00, "Nikon", "AF Nikkor 24mm f/2.8"},
            {0x0A,0x48,0x8E,0x8E,0x24,0x24,0x03,0x00, "Nikon", "AF Nikkor 300mm f/2.8 IF-ED"},
            {0x0B,0x48,0x7C,0x7C,0x24,0x24,0x05,0x00, "Nikon", "AF Nikkor 180mm f/2.8 IF-ED"},
            {0x0D,0x40,0x44,0x72,0x2C,0x34,0x07,0x00, "Nikon", "AF Zoom-Nikkor 35-135mm f/3.5-4.5"},
            {0x0E,0x48,0x5C,0x81,0x30,0x30,0x05,0x00, "Nikon", "AF Zoom-Nikkor 70-210mm f/4"},
            {0x0F,0x58,0x50,0x50,0x14,0x14,0x05,0x00, "Nikon", "AF Nikkor 50mm f/1.8 N"},
            {0x10,0x48,0x8E,0x8E,0x30,0x30,0x08,0x00, "Nikon", "AF Nikkor 300mm f/4 IF-ED"},
            {0x11,0x48,0x44,0x5C,0x24,0x24,0x08,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8"},
            {0x12,0x48,0x5C,0x81,0x30,0x3C,0x09,0x00, "Nikon", "AF Nikkor 70-210mm f/4-5.6"},
            {0x13,0x42,0x37,0x50,0x2A,0x34,0x0B,0x00, "Nikon", "AF Zoom-Nikkor 24-50mm f/3.3-4.5"},
            {0x14,0x48,0x60,0x80,0x24,0x24,0x0B,0x00, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8 ED"},
            {0x15,0x4C,0x62,0x62,0x14,0x14,0x0C,0x00, "Nikon", "AF Nikkor 85mm f/1.8"},
            {0x17,0x3C,0xA0,0xA0,0x30,0x30,0x11,0x00, "Nikon", "Nikkor 500mm f/4 P"},
            {0x18,0x40,0x44,0x72,0x2C,0x34,0x0E,0x00, "Nikon", "AF Zoom-Nikkor 35-135mm f/3.5-4.5 N"},
            {0x1A,0x54,0x44,0x44,0x18,0x18,0x11,0x00, "Nikon", "AF Nikkor 35mm f/2"},
            {0x1B,0x44,0x5E,0x8E,0x34,0x3C,0x10,0x00, "Nikon", "AF Zoom-Nikkor 75-300mm f/4.5-5.6"},
            {0x1C,0x48,0x30,0x30,0x24,0x24,0x12,0x00, "Nikon", "AF Nikkor 20mm f/2.8"},
            {0x1D,0x42,0x44,0x5C,0x2A,0x34,0x12,0x00, "Nikon", "AF Zoom-Nikkor 35-70mm f/3.3-4.5 N"},
            {0x1E,0x54,0x56,0x56,0x24,0x24,0x13,0x00, "Nikon", "AF Micro-Nikkor 60mm f/2.8"},
            {0x20,0x48,0x60,0x80,0x24,0x24,0x15,0x00, "Nikon", "AF Zoom-Nikkor ED 80-200mm f/2.8"},
            {0x22,0x48,0x72,0x72,0x18,0x18,0x16,0x00, "Nikon", "AF DC-Nikkor 135mm f/2"},
            {0x24,0x48,0x60,0x80,0x24,0x24,0x1A,0x02, "Nikon", "AF Zoom-Nikkor ED 80-200mm f/2.8D"},
            {0x25,0x48,0x44,0x5c,0x24,0x24,0x1B,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D"},
            {0x25,0x48,0x44,0x5c,0x24,0x24,0x52,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D N"},
            {0x27,0x48,0x8E,0x8E,0x24,0x24,0xF2,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED"},
            {0x27,0x48,0x8E,0x8E,0x24,0x24,0x1D,0x02, "Nikon", "AF-I Nikkor 300mm f/2.8D IF-ED"},
            {0x28,0x3C,0xA6,0xA6,0x30,0x30,0x1D,0x02, "Nikon", "AF-I Nikkor 600mm f/4D IF-ED"},
            {0x2A,0x54,0x3C,0x3C,0x0C,0x0C,0x26,0x02, "Nikon", "AF Nikkor 28mm f/1.4D"},
            {0x2C,0x48,0x6A,0x6A,0x18,0x18,0x27,0x02, "Nikon", "AF DC-Nikkor 105mm f/2D"},
            {0x2D,0x48,0x80,0x80,0x30,0x30,0x21,0x02, "Nikon", "AF Micro-Nikkor 200mm f/4D IF-ED"},
            {0x2E,0x48,0x5C,0x82,0x30,0x3C,0x28,0x02, "Nikon", "AF Nikkor 70-210mm f/4-5.6D"},
            {0x2F,0x48,0x30,0x44,0x24,0x24,0x29,0x02, "Nikon", "AF Zoom-Nikkor 20-35mm f/2.8(IF)"},
            {0x31,0x54,0x56,0x56,0x24,0x24,0x25,0x02, "Nikon", "AF Micro-Nikkor 60mm f/2.8D"},
            {0x32,0x54,0x6A,0x6A,0x24,0x24,0x35,0x02, "Nikon", "AF Micro-Nikkor 105mm f/2.8D"},
            {0x33,0x48,0x2D,0x2D,0x24,0x24,0x31,0x02, "Nikon", "AF Nikkor 18mm f/2.8D"},
            {0x34,0x48,0x29,0x29,0x24,0x24,0x32,0x02, "Nikon", "AF Fisheye Nikkor 16mm f/2.8D"},
            {0x36,0x48,0x37,0x37,0x24,0x24,0x34,0x02, "Nikon", "AF Nikkor 24mm f/2.8D"},
            {0x37,0x48,0x30,0x30,0x24,0x24,0x36,0x02, "Nikon", "AF Nikkor 20mm f/2.8D"},
            {0x38,0x4C,0x62,0x62,0x14,0x14,0x37,0x02, "Nikon", "AF Nikkor 85mm f/1.8D"},
            {0x3B,0x48,0x44,0x5C,0x24,0x24,0x3A,0x02, "Nikon", "AF Zoom-Nikkor 35-70mm f/2.8D N"},
            {0x3D,0x3C,0x44,0x60,0x30,0x3C,0x3E,0x02, "Nikon", "AF Zoom-Nikkor 35-80mm f/4-5.6D"},
            {0x3E,0x48,0x3C,0x3C,0x24,0x24,0x3D,0x02, "Nikon", "AF Nikkor 28mm f/2.8D"},
            {0x41,0x48,0x7c,0x7c,0x24,0x24,0x43,0x02, "Nikon", "AF Nikkor 180mm f/2.8D IF-ED"},
            {0x42,0x54,0x44,0x44,0x18,0x18,0x44,0x02, "Nikon", "AF Nikkor 35mm f/2D"},
            {0x43,0x54,0x50,0x50,0x0C,0x0C,0x46,0x02, "Nikon", "AF Nikkor 50mm f/1.4D"},
            {0x46,0x3C,0x44,0x60,0x30,0x3C,0x49,0x02, "Nikon", "AF Zoom-Nikkor 35-80mm f/4-5.6D N"},
            {0x47,0x42,0x37,0x50,0x2A,0x34,0x4A,0x02, "Nikon", "AF Zoom-Nikkor 24-50mm f/3.3-4.5D"},
            {0x48,0x48,0x8E,0x8E,0x24,0x24,0x4B,0x02, "Nikon", "AF-S Nikkor 300mm f/2.8D IF-ED"},
            {0x4A,0x54,0x62,0x62,0x0C,0x0C,0x4D,0x02, "Nikon", "AF Nikkor 85mm f/1.4D IF"},
            {0x4C,0x40,0x37,0x6E,0x2C,0x3C,0x4F,0x02, "Nikon", "AF Zoom-Nikkor 24-120mm f/3.5-5.6D IF"},
            {0x4D,0x40,0x3C,0x80,0x2C,0x3C,0x62,0x02, "Nikon", "AF Zoom-Nikkor 28-200mm f/3.5-5.6D IF"},
            {0x4E,0x48,0x72,0x72,0x18,0x18,0x51,0x02, "Nikon", "AF DC-Nikkor 135mm f/2D"},
            {0x4F,0x40,0x37,0x5C,0x2C,0x3C,0x53,0x06, "Nikon", "IX-Nikkor 24-70mm f/3.5-5.6"},
            {0x53,0x48,0x60,0x80,0x24,0x24,0x60,0x02, "Nikon", "AF Zoom-Nikkor 80-200mm f/2.8D ED"},
            {0x54,0x44,0x5C,0x7C,0x34,0x3C,0x58,0x02, "Nikon", "AF Zoom-Micro Nikkor 70-180mm f/4.5-5.6D ED"},
            {0x56,0x48,0x5C,0x8E,0x30,0x3C,0x5A,0x02, "Nikon", "AF Zoom-Nikkor 70-300mm f/4-5.6D ED"},
            {0x59,0x48,0x98,0x98,0x24,0x24,0x5D,0x02, "Nikon", "AF-S Nikkor 400mm f/2.8D IF-ED"},
            {0x5A,0x3C,0x3E,0x56,0x30,0x3C,0x5E,0x06, "Nikon", "IX-Nikkor 30-60mm f/4-5.6"},
            {0x5D,0x48,0x3C,0x5C,0x24,0x24,0x63,0x02, "Nikon", "AF-S Zoom-Nikkor 28-70mm f/2.8D IF-ED"},
            {0x5E,0x48,0x60,0x80,0x24,0x24,0x64,0x02, "Nikon", "AF-S Zoom-Nikkor 80-200mm f/2.8D IF-ED"},
            {0x5F,0x40,0x3C,0x6A,0x2C,0x34,0x65,0x02, "Nikon", "AF Zoom-Nikkor 28-105mm f/3.5-4.5D IF"},
            {0x61,0x44,0x5E,0x86,0x34,0x3C,0x67,0x02, "Nikon", "AF Zoom-Nikkor 75-240mm f/4.5-5.6D"},
            {0x63,0x48,0x2B,0x44,0x24,0x24,0x68,0x02, "Nikon", "AF-S Nikkor 17-35mm f/2.8D IF-ED"},
            {0x64,0x00,0x62,0x62,0x24,0x24,0x6A,0x02, "Nikon", "PC Micro-Nikkor 85mm f/2.8D"},
            {0x65,0x44,0x60,0x98,0x34,0x3C,0x6B,0x0A, "Nikon", "AF VR Zoom-Nikkor 80-400mm f/4.5-5.6D ED"},
            {0x66,0x40,0x2D,0x44,0x2C,0x34,0x6C,0x02, "Nikon", "AF Zoom-Nikkor 18-35mm f/3.5-4.5D IF-ED"},
            {0x67,0x48,0x37,0x62,0x24,0x30,0x6D,0x02, "Nikon", "AF Zoom-Nikkor 24-85mm f/2.8-4D IF"},
            {0x68,0x42,0x3C,0x60,0x2A,0x3C,0x6E,0x06, "Nikon", "AF Zoom-Nikkor 28-80mm f/3.3-5.6G"},
            {0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x06, "Nikon", "AF Zoom-Nikkor 70-300mm f/4-5.6G"},
            {0x6A,0x48,0x8E,0x8E,0x30,0x30,0x70,0x02, "Nikon", "AF-S Nikkor 300mm f/4D IF-ED"},
            {0x6B,0x48,0x24,0x24,0x24,0x24,0x71,0x02, "Nikon", "AF Nikkor ED 14mm f/2.8D"},
            {0x6D,0x48,0x8E,0x8E,0x24,0x24,0x73,0x02, "Nikon", "AF-S Nikkor 300mm f/2.8D IF-ED II"},
            {0x6E,0x48,0x98,0x98,0x24,0x24,0x74,0x02, "Nikon", "AF-S Nikkor 400mm f/2.8D IF-ED II"},
            {0x6F,0x3C,0xA0,0xA0,0x30,0x30,0x75,0x02, "Nikon", "AF-S Nikkor 500mm f/4D IF-ED"},
            {0x70,0x3C,0xA6,0xA6,0x30,0x30,0x76,0x02, "Nikon", "AF-S Nikkor 600mm f/4D IF-ED"},
            {0x72,0x48,0x4C,0x4C,0x24,0x24,0x77,0x00, "Nikon", "Nikkor 45mm f/2.8 P"},
            {0x74,0x40,0x37,0x62,0x2C,0x34,0x78,0x06, "Nikon", "AF-S Zoom-Nikkor 24-85mm f/3.5-4.5G IF-ED"},
            {0x75,0x40,0x3C,0x68,0x2C,0x3C,0x79,0x06, "Nikon", "AF Zoom-Nikkor 28-100mm f/3.5-5.6G"},
            {0x76,0x58,0x50,0x50,0x14,0x14,0x7A,0x02, "Nikon", "AF Nikkor 50mm f/1.8D"},
            {0x77,0x48,0x5C,0x80,0x24,0x24,0x7B,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED"},
            {0x78,0x40,0x37,0x6E,0x2C,0x3C,0x7C,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 24-120mm f/3.5-5.6G IF-ED"},
            {0x79,0x40,0x3C,0x80,0x2C,0x3C,0x7F,0x06, "Nikon", "AF Zoom-Nikkor 28-200mm f/3.5-5.6G IF-ED"},
            {0x7A,0x3C,0x1F,0x37,0x30,0x30,0x7E,0x06, "Nikon", "AF-S DX Zoom-Nikkor 12-24mm f/4G IF-ED"},
            {0x7B,0x48,0x80,0x98,0x30,0x30,0x80,0x0E, "Nikon", "AF-S VR Zoom-Nikkor 200-400mm f/4G IF-ED"},
            {0x7D,0x48,0x2B,0x53,0x24,0x24,0x82,0x06, "Nikon", "AF-S DX Zoom-Nikkor 17-55mm f/2.8G IF-ED"},
            {0x7F,0x40,0x2D,0x5C,0x2C,0x34,0x84,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-70mm f/3.5-4.5G IF-ED"},
            {0x80,0x48,0x1A,0x1A,0x24,0x24,0x85,0x06, "Nikon", "AF DX Fisheye-Nikkor 10.5mm f/2.8G ED"},
            {0x81,0x54,0x80,0x80,0x18,0x18,0x86,0x0E, "Nikon", "AF-S VR Nikkor 200mm f/2G IF-ED"},
            {0x82,0x48,0x8E,0x8E,0x24,0x24,0x87,0x0E, "Nikon", "AF-S VR Nikkor 300mm f/2.8G IF-ED"},
            {0x89,0x3C,0x53,0x80,0x30,0x3C,0x8B,0x06, "Nikon", "AF-S DX Zoom-Nikkor 55-200mm f/4-5.6G ED"},
            {0x8A,0x54,0x6A,0x6A,0x24,0x24,0x8C,0x0E, "Nikon", "AF-S VR Micro-Nikkor 105mm f/2.8G IF-ED"},
            {0x8B,0x40,0x2D,0x80,0x2C,0x3C,0x8D,0x0E, "Nikon", "AF-S DX VR Zoom-Nikkor 18-200mm f/3.5-5.6G IF-ED"},
            {0x8C,0x40,0x2D,0x53,0x2C,0x3C,0x8E,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-55mm f/3.5-5.6G ED"},
            {0x8F,0x40,0x2D,0x72,0x2C,0x3C,0x91,0x06, "Nikon", "AF-S DX Zoom-Nikkor 18-135mm f/3.5-5.6G IF-ED"},
            //#"?? ?? 5C 8E 34 3C ??" "0E" "Nikon" "AF-S VR Zoom-Nikkor 70-300mm f/4.5-5.6G IF-ED";
            //#
            {0x06,0x3F,0x68,0x68,0x2C,0x2C,0x06,0x00, "Cosina", "100mm F/3.5 Macro"},
            //#
            {0x26,0x48,0x11,0x11,0x30,0x30,0x1C,0x02, "Sigma", "8mm F4 EX Circular Fisheye"},
            {0x48,0x3C,0x19,0x31,0x30,0x3C,0x4B,0x06, "Sigma", "10-20mm F4-5.6 EX DC HSM"},
            {0x48,0x38,0x1F,0x37,0x34,0x3C,0x4B,0x06, "Sigma", "12-24mm F4.5-5.6 EX Aspherical DG HSM"},
            {0x02,0x3F,0x24,0x24,0x2C,0x2C,0x02,0x00, "Sigma", "14mm F3.5"},
            {0x48,0x48,0x24,0x24,0x24,0x24,0x4B,0x02, "Sigma", "14mm F2.8 EX ASPHERICAL HSM"},
            {0x26,0x48,0x27,0x27,0x24,0x24,0x1C,0x02, "Sigma", "15mm F2.8 EX Diagonal Fish-Eye"},
            {0x26,0x40,0x27,0x3F,0x2C,0x34,0x1C,0x02, "Sigma", "15-30mm F3.5-4.5 EX Aspherical DG DF"},
            {0x48,0x48,0x2B,0x44,0x24,0x30,0x4B,0x06, "Sigma", "17-35mm F2.8-4 EX DG  Aspherical HSM"},
            {0x26,0x54,0x2B,0x44,0x24,0x30,0x1C,0x02, "Sigma", "17-35mm F2.8-4 EX ASPHERICAL"},
            {0x26,0x48,0x2D,0x50,0x24,0x24,0x1C,0x06, "Sigma", "18-50mm F2.8 EX DC"},
            {0x26,0x40,0x2D,0x50,0x2C,0x3C,0x1C,0x06, "Sigma", "18-50mm F3.5-5.6 DC"},
            {0x26,0x40,0x2D,0x70,0x2B,0x3C,0x1C,0x06, "Sigma", "18-125mm F3.5-5.6 DC"},
            {0x26,0x40,0x2D,0x80,0x2C,0x40,0x1C,0x06, "Sigma", "18-200mm F3.5-6.3 DC"},
            {0x26,0x58,0x31,0x31,0x14,0x14,0x1C,0x02, "Sigma", "20mm F1.8 EX Aspherical DG DF RF"},
            {0x26,0x58,0x37,0x37,0x14,0x14,0x1C,0x02, "Sigma", "24mm F1.8 EX Aspherical DG DF MACRO"},
            {0x02,0x46,0x37,0x37,0x25,0x25,0x02,0x00, "Sigma", "24mm F2.8 Macro"},
            {0x26,0x48,0x37,0x56,0x24,0x24,0x1C,0x02, "Sigma", "24-60mm F2.8 EX DG"},
            {0x26,0x54,0x37,0x5C,0x24,0x24,0x1C,0x02, "Sigma", "24-70mm F2.8 EX DG Macro"},
            {0x26,0x40,0x37,0x5C,0x2C,0x3C,0x1C,0x02, "Sigma", "24-70mm F3.5-5.6 ASPHERICAL HF"},
            {0x26,0x54,0x37,0x73,0x24,0x34,0x1C,0x02, "Sigma", "24-135mm F2.8-4.5"},
            {0x26,0x48,0x3C,0x5C,0x24,0x24,0x1C,0x06, "Sigma", "28-70mm F2.8 EX DG"},
            {0x26,0x48,0x3C,0x5C,0x24,0x30,0x1C,0x02, "Sigma", "28-70mm F2.8-4 HIGH SPEED ZOOM"},
            {0x02,0x46,0x3C,0x5C,0x25,0x25,0x02,0x00, "Sigma", "28-70mm F2.8"},
            {0x02,0x3F,0x3C,0x5C,0x2D,0x35,0x02,0x00, "Sigma", "28-70mm F3.5-4.5 UC"},
            {0x26,0x40,0x3C,0x60,0x2C,0x3C,0x1C,0x02, "Sigma", "28-80mm F3.5-5.6 Mini Zoom Macro II Aspherical"},
            {0x26,0x3E,0x3C,0x6A,0x2E,0x3C,0x1C,0x02, "Sigma", "28-105mm F3.8-5.6 UC-III ASPHERICAL IF"},
            {0x26,0x40,0x3C,0x80,0x2C,0x3C,0x1C,0x02, "Sigma", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
            {0x26,0x40,0x3C,0x80,0x2B,0x3C,0x1C,0x02, "Sigma", "28-200mm F3.5-5.6 Compact Aspherical Hyperzoom Macro"},
            {0x26,0x41,0x3C,0x8E,0x2C,0x40,0x1C,0x02, "Sigma", "28-300mm F3.5-6.3 DG MACRO"},
            {0x26,0x40,0x3C,0x8E,0x2C,0x40,0x1C,0x02, "Sigma", "28-300mm F3.5-6.3 Macro"},
            {0x48,0x54,0x3E,0x3E,0x0C,0x0C,0x4B,0x06, "Sigma", "30mm F1.4 EX DC HSM"},
            {0x02,0x40,0x44,0x73,0x2B,0x36,0x02,0x00, "Sigma", "35-135mm F3.5-4.5 a"},
            {0x32,0x54,0x50,0x50,0x24,0x24,0x35,0x02, "Sigma", "50mm F2.8 EX DG Macro"},
            {0x48,0x3C,0x50,0xA0,0x30,0x40,0x4B,0x02, "Sigma", "50-500mmF4-6.3 EX APO RF HSM"},
            {0x26,0x3C,0x54,0x80,0x30,0x3C,0x1C,0x06, "Sigma", "55-200mm F4-5.6 DC"},
            {0x48,0x54,0x5C,0x80,0x24,0x24,0x4B,0x02, "Sigma", "70-200mm F2.8 EX APO IF HSM"},
            {0x26,0x3C,0x5C,0x8E,0x30,0x3C,0x1C,0x02, "Sigma", "70-300mm F4-5.6 DG MACRO"},
            {0x56,0x3C,0x5C,0x8E,0x30,0x3C,0x1C,0x02, "Sigma", "70-300mm F4-5.6 APO Macro Super II"},
            {0x02,0x37,0x5E,0x8E,0x35,0x3D,0x02,0x00, "Sigma", "75-300mm F4.5-5.6 APO"},
            {0x02,0x48,0x65,0x65,0x24,0x24,0x02,0x00, "Sigma", "90mm F2.8 Macro"},
            {0x77,0x44,0x61,0x98,0x34,0x3C,0x7B,0x0E, "Sigma", "80-400mm f4.5-5.6 EX OS"},
            {0x48,0x48,0x68,0x8E,0x30,0x30,0x4B,0x02, "Sigma", "100-300mm F4 EX IF HSM"},
            {0x26,0x44,0x73,0x98,0x34,0x3C,0x1C,0x02, "Sigma", "135-400mm F4.5-5.6 APO Aspherical"},
            {0x48,0x48,0x76,0x76,0x24,0x24,0x4B,0x06, "Sigma", "150mm F2.8 EX DG APO Macro HSM"},
            {0x26,0x40,0x7B,0xA0,0x34,0x40,0x1C,0x02, "Sigma", "APO 170-500mm F5-6.3 ASPHERICAL RF"},
            {0x48,0x4C,0x7D,0x7D,0x2C,0x2C,0x4B,0x02, "Sigma", "APO MACRO 180mm F3.5 EX DG HSM"},
            {0x48,0x54,0x8E,0x8E,0x24,0x24,0x4B,0x02, "Sigma", "APO 300mm F2.8 EX DG HSM"},
            {0x26,0x48,0x8E,0x8E,0x30,0x30,0x1C,0x02, "Sigma", "APO TELE MACRO 300mm F4"},
            {0x02,0x2F,0x98,0x98,0x3D,0x3D,0x02,0x00, "Sigma", "400mm F5.6 APO"},
            //#
            {0x03,0x43,0x5C,0x81,0x35,0x35,0x02,0x00, "Soligor", "AF C/D ZOOM UMCS 70-210mm 1:4.5"},
            //#
            {0x00,0x3C,0x1F,0x37,0x30,0x30,0x00,0x06, "Tokina", "AT-X 124 AF PRO DX - AF 12-24mm f/4"},
            {0x00,0x40,0x2B,0x2B,0x2C,0x2C,0x00,0x02, "Tokina", "AT-X 17 AF PRO - AF 17mm f/3.5"},
            {0x25,0x48,0x3C,0x5C,0x24,0x24,0x1B,0x02, "Tokina", "AT-X287AF PRO SV 28-70mm F2.8"},
            {0x00,0x48,0x3C,0x60,0x24,0x24,0x00,0x02, "Tokina", "AT-X280AF PRO 28-80mm F2.8 ASPHERICAL"},
            {0x14,0x54,0x60,0x80,0x24,0x24,0x0B,0x00, "Tokina", "AT-X828AF 80-200mm F2.8"},
            {0x24,0x44,0x60,0x98,0x34,0x3C,0x1A,0x02, "Tokina", "AT-X840AF II 80-400mm F4.5-5.6"},
            {0x00,0x54,0x68,0x68,0x24,0x24,0x00,0x02, "Tokina", "AT-X M100 PRO D - 100mm F2.8"},
            {0x14,0x48,0x68,0x8E,0x30,0x30,0x0B,0x00, "Tokina", "AT-X340AF II 100-300mm F4"},
            {0x00,0x54,0x8E,0x8E,0x24,0x24,0x00,0x02, "Tokina", "AT-X300AF PRO 300mm F2.8"},
            //#
            {0x00,0x36,0x1C,0x2D,0x34,0x3C,0x00,0x06, "Tamron", "SP AF11-18mm F/4.5-5.6 Di II LD Aspherical (IF)"},
            {0x07,0x46,0x2B,0x44,0x24,0x30,0x03,0x02, "Tamron", "SP AF17-35mm F/2.8-4 Di LD Aspherical (IF)"},
            {0x00,0x3F,0x2D,0x80,0x2B,0x40,0x00,0x06, "Tamron", "AF18-200mm f/3.5-6.3 XR Di II LD Aspherical (IF)"},
            {0x00,0x3F,0x2D,0x80,0x2C,0x40,0x00,0x06, "Tamron", "AF18-200mm f/3.5-6.3 XR Di II LD Aspherical (IF) MACRO"},
            {0x07,0x40,0x30,0x45,0x2D,0x35,0x03,0x02, "Tamron", "AF19-35mm F3.5-4.5"},
            {0x45,0x41,0x37,0x72,0x2C,0x3C,0x48,0x02, "Tamron", "SP AF24-135mm F3.5-5.6 AD ASPHERICAL(IF)MACRO"},
            {0x33,0x54,0x3C,0x5E,0x24,0x24,0x62,0x02, "Tamron", "SP AF28-75mm F2.8 XR Di LD ASPHERICAL(IF)MACRO"},
            {0x10,0x3D,0x3C,0x60,0x2C,0x3C,0xD2,0x02, "Tamron", "AF28-80mm f/3.5-5.6 Aspherical"},
            {0x00,0x48,0x3C,0x6A,0x24,0x24,0x00,0x02, "Tamron", "SP AF28-105mm f/2.8"},
            {0x0B,0x3E,0x3D,0x7F,0x2F,0x3D,0x0E,0x02, "Tamron", "AF28-200mm f/3.8-5.6D"},
            {0x0B,0x3E,0x3D,0x7F,0x2F,0x3D,0x0E,0x00, "Tamron", "AF28-200mm f/3.8-5.6"},
            {0x4D,0x41,0x3C,0x8E,0x2B,0x40,0x62,0x02, "Tamron", "AF28-300mm f/3.5-6.3 XR Di LD Aspherical (IF)"},
            {0x4D,0x41,0x3C,0x8E,0x2C,0x40,0x62,0x02, "Tamron", "AF28-300mm f/3.5-6.3D"},
            {0x69,0x48,0x5C,0x8E,0x30,0x3C,0x6F,0x02, "Tamron", "AF70-300mm F4-5.6 LD MACRO 1:2"},
            {0x32,0x53,0x64,0x64,0x24,0x24,0x35,0x02, "Tamron", "SP AF90mm f/2.8 Di 1:1 Macro"},
            {0x00,0x4C,0x7C,0x7C,0x2C,0x2C,0x00,0x02, "Tamron", "SP AF180mm F3.5 Di Model B01"},
            {0x20,0x3C,0x80,0x98,0x3D,0x3D,0x1E,0x02, "Tamron", "AF200-400mm f/5.6 LD IF"},
            {0x00,0x3E,0x80,0xA0,0x38,0x3F,0x00,0x02, "Tamron", "SP AF200-500mm f/5-6.3 Di LD (IF)"},
            {0x00,0x3F,0x80,0xA0,0x38,0x3F,0x00,0x02, "Tamron", "SP AF200-500mm F5-6.3 Di"},
            //#
            {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01, "Manual Lens", "No CPU"},
            {0x1E,0x5D,0x64,0x64,0x20,0x20,0x13,0x00, "Unknown", "90mm F/2.5"},
            {0x2F,0x40,0x30,0x44,0x2C,0x34,0x29,0x02, "Unknown", "20-35mm F/3.5-4.5D"},
            {0x12,0x3B,0x68,0x8D,0x3D,0x43,0x09,0x02, "Unknown", "100-290mm F5.6-6.7"},
            //#
            {0,0,0,0,0,0,0,0, NULL, NULL}
        };

        if (value.typeId() != undefined) return os << value;

        DataBuf lens(value.size());
        // ByteOrder is only to satisfy the interface
        value.copy(lens.pData_, invalidByteOrder);

        int idx = 0;
        if (0 == memcmp(lens.pData_, "0100", 4)) {
            idx = 6;
        }
        else if (0 == memcmp(lens.pData_, "0101", 4)) {
            idx = 11;
        }
        else if (0 == memcmp(lens.pData_, "0201", 4)) {
            // Here we should decrypt(lens.pData_ + 4, lens.size_ - 4);
            // however, the decrypt algorithm requires access to serial number
            // and shutter count tags but print functions are static...
            idx = 11;
        }
        if (idx == 0 || lens.size_ < idx + 7) {
            // Unknown version or not enough data
            return os << value;
        }
        for (int i = 0; fmountlens[i].lensname != NULL; ++i) {
            if (   lens.pData_[idx]   == fmountlens[i].lid
                && lens.pData_[idx+1] == fmountlens[i].stps
                && lens.pData_[idx+2] == fmountlens[i].focs
                && lens.pData_[idx+3] == fmountlens[i].focl
                && lens.pData_[idx+4] == fmountlens[i].aps
                && lens.pData_[idx+5] == fmountlens[i].apl
                && lens.pData_[idx+6] == fmountlens[i].lfw) {
                // Lens found in database
                return os << fmountlens[i].manuf << " " << fmountlens[i].lensname;
            }
        }
        // Lens not found in database
        return os << value;
#else
        return os << value;
#endif // EXV_HAVE_LENSDATA
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createNikonMakerNote(bool        alloc,
                                            const byte* buf,
                                            long        len,
                                            ByteOrder   /*byteOrder*/,
                                            long        /*offset*/)
    {
        // If there is no "Nikon" string it must be Nikon1 format
        if (len < 6 || std::string(reinterpret_cast<const char*>(buf), 6)
                    != std::string("Nikon\0", 6)) {
            return MakerNote::AutoPtr(new Nikon1MakerNote(alloc));
        }
        // If the "Nikon" string is not followed by a TIFF header, we assume
        // Nikon2 format
        TiffHeader tiffHeader;
        if (   len < 18
            || tiffHeader.read(buf + 10) != 0 || tiffHeader.tag() != 0x002a) {
            return MakerNote::AutoPtr(new Nikon2MakerNote(alloc));
        }
        // Else we have a Nikon3 makernote
        return MakerNote::AutoPtr(new Nikon3MakerNote(alloc));
    }

}                                       // namespace Exiv2
