// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tags.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Jan-04, ahu: created
             21-Jan-05, ahu: added MakerNote TagInfo registry and related code
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "error.hpp"
#include "types.hpp"
#include "ifd.hpp"
#include "value.hpp"
#include "makernote.hpp"
#include "mn.hpp"                // To ensure that all makernotes are registered

#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <cassert>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name, const char* item)
        : ifdId_(ifdId), name_(name), item_(item)
    {
    }

    // Todo: Allow to register new IfdInfo entries from elsewhere (the makernotes)
    // Important: IFD item must be unique!
    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(ifdIdNotSet, "(Unknown IFD)", "(Unknown item)"),
        IfdInfo(ifd0Id, "IFD0", "Image"),
        IfdInfo(exifIfdId, "Exif", "Photo"),  // just to avoid 'Exif.Exif.*' keys
        IfdInfo(gpsIfdId, "GPSInfo", "GPSInfo"),
        IfdInfo(iopIfdId, "Iop", "Iop"),
        IfdInfo(ifd1Id, "IFD1", "Thumbnail"),
        IfdInfo(canonIfdId, "Makernote", "Canon"),
        IfdInfo(canonCsIfdId, "Makernote", "CanonCs"),
        IfdInfo(canonSiIfdId, "Makernote", "CanonSi"),
        IfdInfo(canonCfIfdId, "Makernote", "CanonCf"),
        IfdInfo(canonPiIfdId, "Makernote", "CanonPi"),
        IfdInfo(canonPaIfdId, "Makernote", "CanonPa"),
        IfdInfo(fujiIfdId, "Makernote", "Fujifilm"),
        IfdInfo(minoltaIfdId, "Makernote", "Minolta"),
        IfdInfo(minoltaCs5DIfdId, "Makernote", "MinoltaCs5D"),
        IfdInfo(minoltaCs7DIfdId, "Makernote", "MinoltaCs7D"),
        IfdInfo(minoltaCsOldIfdId, "Makernote", "MinoltaCsOld"),
        IfdInfo(minoltaCsNewIfdId, "Makernote", "MinoltaCsNew"),
        IfdInfo(nikon1IfdId, "Makernote", "Nikon1"),
        IfdInfo(nikon2IfdId, "Makernote", "Nikon2"),
        IfdInfo(nikon3IfdId, "Makernote", "Nikon3"),
        IfdInfo(olympusIfdId, "Makernote", "Olympus"),
        IfdInfo(panasonicIfdId, "Makernote", "Panasonic"),
        IfdInfo(sigmaIfdId, "Makernote", "Sigma"),
        IfdInfo(sonyIfdId, "Makernote", "Sony"),
        IfdInfo(lastIfdId, "(Last IFD info)", "(Last IFD item)")
    };

    SectionInfo::SectionInfo(
        SectionId sectionId,
        const char* name,
        const char* desc
    )
        : sectionId_(sectionId), name_(name), desc_(desc)
    {
    }

    const SectionInfo ExifTags::sectionInfo_[] = {
        SectionInfo(sectionIdNotSet, "(UnknownSection)", "Unknown section"),
        SectionInfo(imgStruct, "ImageStructure", "Image data structure"),
        SectionInfo(recOffset, "RecordingOffset", "Recording offset"),
        SectionInfo(imgCharacter, "ImageCharacteristics", "Image data characteristics"),
        SectionInfo(otherTags, "OtherTags", "Other data"),
        SectionInfo(exifFormat, "ExifFormat", "Exif data structure"),
        SectionInfo(exifVersion, "ExifVersion", "Exif Version"),
        SectionInfo(imgConfig, "ImageConfig", "Image configuration"),
        SectionInfo(userInfo, "UserInfo", "User information"),
        SectionInfo(relatedFile, "RelatedFile", "Related file"),
        SectionInfo(dateTime, "DateTime", "Date and time"),
        SectionInfo(captureCond, "CaptureConditions", "Picture taking conditions"),
        SectionInfo(gpsTags, "GPS", "GPS information"),
        SectionInfo(iopTags, "Interoperability", "Interoperability information"),
        SectionInfo(makerTags, "Makernote", "Vendor specific information"),
        SectionInfo(lastSectionId, "(LastSection)", "Last section")
    };

    TagInfo::TagInfo(
        uint16_t tag,
        const char* name,
        const char* title,
        const char* desc,
        IfdId ifdId,
        SectionId sectionId,
        TypeId typeId,
        PrintFct printFct
    )
        : tag_(tag), name_(name), title_(title), desc_(desc), ifdId_(ifdId),
          sectionId_(sectionId), typeId_(typeId), printFct_(printFct)
    {
    }

    //! NewSubfileType, TIFF tag 0x00fe - this is actually a bitmask
    extern const TagDetails exifNewSubfileType[] = {
        {  0, "Primary image"                                               },
        {  1, "Thumbnail/Preview image"                                     },
        {  2, "Primary image, Multi page file"                              },
        {  3, "Thumbnail/Preview image, Multi page file"                    },
        {  4, "Primary image, Transparency mask"                            },
        {  5, "Thumbnail/Preview image, Transparency mask"                  },
        {  6, "Primary image, Multi page file, Transparency mask"           },
        {  7, "Thumbnail/Preview image, Multi page file, Transparency mask" }
    };

    //! Units for measuring X and Y resolution, tags 0x0128, 0xa210
    extern const TagDetails exifUnit[] = {
        { 1, "none" },
        { 2, "inch" },
        { 3, "cm"   }
    };

    //! Compression, tag 0x0103
    extern const TagDetails exifCompression[] = {
        {     1, "Uncompressed"             },
        {     2, "CCITT RLE"                },
        {     3, "T4/Group 3 Fax"           },
        {     4, "T6/Group 4 Fax"           },
        {     5, "LZW"                      },
        {     6, "JPEG (old-style)"         },
        {     7, "JPEG"                     },
        {     8, "Adobe Deflate"            },
        {     9, "JBIG B&W"                 },
        {    10, "JBIG Color"               },
        { 32766, "Next 2-bits RLE"          },
        { 32771, "CCITT RLE 1-word"         },
        { 32773, "PackBits (Macintosh RLE)" },
        { 32809, "Thunderscan RLE"          },
        { 32895, "IT8 CT Padding"           },
        { 32896, "IT8 Linework RLE"         },
        { 32897, "IT8 Monochrome Picture"   },
        { 32898, "IT8 Binary Lineart"       },
        { 32908, "Pixar Film (10-bits LZW)" },
        { 32909, "Pixar Log (11-bits ZIP)"  },
        { 32946, "Pixar Deflate"            },
        { 32947, "Kodak DCS Encoding"       },
        { 34661, "ISO JBIG"                 },
        { 34676, "SGI Log Luminance RLE"    },
        { 34677, "SGI Log 24-bits packed"   },
        { 34712, "Leadtools JPEG 2000"      },
        { 34713, "Nikon NEF Compressed"     }
    };

    //! PhotometricInterpretation, tag 0x0106
    extern const TagDetails exifPhotometricInterpretation[] = {
        {     0, "White Is Zero"      },
        {     1, "Black Is Zero"      },
        {     2, "RGB"                },
        {     3, "RGB Palette"        },
        {     4, "Transparency Mask"  },
        {     5, "CMYK"               },
        {     6, "YCbCr"              },
        {     8, "CIELab"             },
        {     9, "ICCLab"             },
        {    10, "ITULab"             },
        { 32803, "Color Filter Array" },
        { 32844, "Pixar LogL"         },
        { 32845, "Pixar LogLuv"       },
        { 34892, "Linear Raw"         }
    };

    //! Orientation, tag 0x0112
    extern const TagDetails exifOrientation[] = {
        { 1, "top, left"     },
        { 2, "top, right"    },
        { 3, "bottom, right" },
        { 4, "bottom, left"  },
        { 5, "left, top"     },
        { 6, "right, top"    },
        { 7, "right, bottom" },
        { 8, "left, bottom"  }
    };

    //! YCbCrPositioning, tag 0x0213
    extern const TagDetails exifYCbCrPositioning[] = {
        { 1, "Centered" },
        { 2, "Co-sited" }
    };

    //! Base IFD Tags (IFD0 and IFD1)
    static const TagInfo ifdTagInfo[] = {
        TagInfo(0x00fe, "NewSubfileType", "New Subfile Type", 
                "A general indication of the kind of data contained in this subfile.",
                ifd0Id, imgStruct, unsignedLong, EXV_PRINT_TAG(exifNewSubfileType)), // TIFF tag
        TagInfo(0x0100, "ImageWidth", "Image Width",
                "The number of columns of image data, equal to the number of "
                "pixels per row. In JPEG compressed data a JPEG marker is "
                "used instead of this tag.",
                ifd0Id, imgStruct, unsignedLong, printValue),
        TagInfo(0x0101, "ImageLength", "Image Length",
                "The number of rows of image data. In JPEG compressed data a "
                "JPEG marker is used instead of this tag.",
                ifd0Id, imgStruct, unsignedLong, printValue),
        TagInfo(0x0102, "BitsPerSample", "Bits per Sample",
                "The number of bits per image component. In this standard each "
                "component of the image is 8 bits, so the value for this "
                "tag is 8. See also <SamplesPerPixel>. In JPEG compressed data "
                "a JPEG marker is used instead of this tag.",
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0103, "Compression", "Compression",
                "The compression scheme used for the image data. When a "
                "primary image is JPEG compressed, this designation is "
                "not necessary and is omitted. When thumbnails use JPEG "
                "compression, this tag value is set to 6.",
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifCompression)),
        TagInfo(0x0106, "PhotometricInterpretation", "Photometric Interpretation", 
                "The pixel composition. In JPEG compressed data a JPEG "
                "marker is used instead of this tag.",
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifPhotometricInterpretation)),
        TagInfo(0x010a, "FillOrder", "Fill Order", 
                "The logical order of bits within a byte", 
                ifd0Id, imgStruct, unsignedShort, printValue), // TIFF tag
        TagInfo(0x010d, "DocumentName", "Document Name", 
                "The name of the document from which this image was scanned", 
                ifd0Id, imgStruct, asciiString, printValue), // TIFF tag
        TagInfo(0x010e, "ImageDescription", "Image Description",
                "A character string giving the title of the image. It may be "
                "a comment such as \"1988 company picnic\" or "
                "the like. Two-bytes character codes cannot be used. "
                "When a 2-bytes code is necessary, the Exif Private tag "
                "<UserComment> is to be used.",
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x010f, "Make", "Manufacturer",
                "The manufacturer of the recording "
                "equipment. This is the manufacturer of the DSC, scanner, "
                "video digitizer or other equipment that generated the "
                "image. When the field is left blank, it is treated as unknown.",
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0110, "Model", "Model",
                "The model name or model number of the equipment. This is the "
                "model name or number of the DSC, scanner, video digitizer "
                "or other equipment that generated the image. When the field "
                "is left blank, it is treated as unknown.",
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0111, "StripOffsets", "Strip Offsets",
                "For each strip, the byte offset of that strip. It is "
                "recommended that this be selected so the number of strip "
                "bytes does not exceed 64 Kbytes. With JPEG compressed "
                "data this designation is not needed and is omitted. See also "
                "<RowsPerStrip> and <StripByteCounts>.",
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0112, "Orientation", "Orientation",
                "The image orientation viewed in terms of rows and columns.",
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifOrientation)),
        TagInfo(0x0115, "SamplesPerPixel", "Samples per Pixel",
                "The number of components per pixel. Since this standard applies "
                "to RGB and YCbCr images, the value set for this tag is 3. "
                "In JPEG compressed data a JPEG marker is used instead of this tag.",
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0116, "RowsPerStrip", "Rows per Strip",
                "The number of rows per strip. This is the number of rows "
                "in the image of one strip when an image is divided into "
                "strips. With JPEG compressed data this designation is not "
                "needed and is omitted. See also <StripOffsets> and <StripByteCounts>.",
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0117, "StripByteCounts", "Strip Byte Count",
                "The total number of bytes in each strip. With JPEG compressed "
                "data this designation is not needed and is omitted.",
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x011a, "XResolution", "x-Resolution",
                "The number of pixels per <ResolutionUnit> in the <ImageWidth> "
                "direction. When the image resolution is unknown, 72 [dpi] is designated.",
                ifd0Id, imgStruct, unsignedRational, printLong),
        TagInfo(0x011b, "YResolution", "y-Resolution",
                "The number of pixels per <ResolutionUnit> in the <ImageLength> "
                "direction. The same value as <XResolution> is designated.",
                ifd0Id, imgStruct, unsignedRational, printLong),
        TagInfo(0x011c, "PlanarConfiguration", "Planar Configuration",
                "Indicates whether pixel components are recorded in a chunky "
                "or planar format. In JPEG compressed files a JPEG marker "
                "is used instead of this tag. If this field does not exist, "
                "the TIFF default of 1 (chunky) is assumed.",
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0128, "ResolutionUnit", "Resolution Unit",
                "The unit for measuring <XResolution> and <YResolution>. The same "
                "unit is used for both <XResolution> and <YResolution>. If "
                "the image resolution is unknown, 2 (inches) is designated.",
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifUnit)),
        TagInfo(0x012d, "TransferFunction", "Transfer Function",
                "A transfer function for the image, described in tabular style. "
                "Normally this tag is not necessary, since color space is "
                "specified in the color space information tag (<ColorSpace>).",
                ifd0Id, imgCharacter, unsignedShort, printValue),
        TagInfo(0x0131, "Software", "Software",
                "This tag records the name and version of the software or "
                "firmware of the camera or image input device used to "
                "generate the image. The detailed format is not specified, but "
                "it is recommended that the example shown below be "
                "followed. When the field is left blank, it is treated as unknown.", 
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0132, "DateTime", "Date and Time",
                "The date and time of image creation. In this standard "
                "(EXIF-2.1) it is the date and time the file was changed.",
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x013b, "Artist", "Artist",
                "This tag records the name of the camera owner, photographer or "
                "image creator. The detailed format is not specified, but it is "
                "recommended that the information be written as in the example "
                "below for ease of Interoperability. When the field is "
                "left blank, it is treated as unknown.",
                ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x013e, "WhitePoint", "White Point",
                "The chromaticity of the white point of the image. Normally "
                "this tag is not necessary, since color space is specified "
                "in the colorspace information tag (<ColorSpace>).",
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x013f, "PrimaryChromaticities", "Primary Chromaticities",
                "The chromaticity of the three primary colors of the image. "
                "Normally this tag is not necessary, since colorspace is "
                "specified in the colorspace information tag (<ColorSpace>).",
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x014a, "SubIFDs", "SubIFD Offsets",
                "Defined by Adobe Corporation to enable TIFF Trees within a TIFF file.",
                ifd0Id, otherTags, unsignedLong, printValue),
        TagInfo(0x0156, "TransferRange", "Transfer Range",
                "Expands the range of the TransferFunction",
                ifd0Id, imgCharacter, unsignedShort, printValue), // TIFF tag
        TagInfo(0x0200, "JPEGProc", "JPEGProc",
                "This field indicates the process used to produce the compressed data",
                ifd0Id, recOffset, unsignedLong, printValue), // TIFF tag
        TagInfo(0x0201, "JPEGInterchangeFormat", "JPEG Interchange Format",
                "The offset to the start byte (SOI) of JPEG compressed "
                "thumbnail data. This is not used for primary image JPEG data.",
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "JPEG Interchange Format Length",
                "The number of bytes of JPEG compressed thumbnail data. This "
                "is not used for primary image JPEG data. JPEG thumbnails "
                "are not divided but are recorded as a continuous JPEG "
                "bitstream from SOI to EOI. Appn and COM markers should "
                "not be recorded. Compressed thumbnails must be recorded in no "
                "more than 64 Kbytes, including all other data to be recorded in APP1.",
                ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0211, "YCbCrCoefficients", "YCbCr Coefficients",
                "The matrix coefficients for transformation from RGB to YCbCr "
                "image data. No default is given in TIFF; but here the "
                "value given in Appendix E, \"Color Space Guidelines\", is used "
                "as the default. The color space is declared in a "
                "color space information tag, with the default being the value "
                "that gives the optimal image characteristics "
                "Interoperability this condition.",
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x0212, "YCbCrSubSampling", "YCbCr Sub-Sampling",
                "The sampling ratio of chrominance components in relation to the "
                "luminance component. In JPEG compressed data a JPEG marker "
                "is used instead of this tag.",
                ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0213, "YCbCrPositioning", "YCbCr Positioning",
                "The position of chrominance components in relation to the "
                "luminance component. This field is designated only for "
                "JPEG compressed data or uncompressed YCbCr data. The TIFF "
                "default is 1 (centered); but when Y:Cb:Cr = 4:2:2 it is "
                "recommended in this standard that 2 (co-sited) be used to "
                "record data, in order to improve the image quality when viewed "
                "on TV systems. When this field does not exist, the reader shall "
                "assume the TIFF default. In the case of Y:Cb:Cr = 4:2:0, the "
                "TIFF default (centered) is recommended. If the reader "
                "does not have the capability of supporting both kinds of "
                "<YCbCrPositioning>, it shall follow the TIFF default regardless "
                "of the value in this field. It is preferable that readers "
                "be able to support both centered and co-sited positioning.",
                ifd0Id, imgStruct, unsignedShort, EXV_PRINT_TAG(exifYCbCrPositioning)),
        TagInfo(0x0214, "ReferenceBlackWhite", "Reference Black/White",
                "The reference black point value and reference white point "
                "value. No defaults are given in TIFF, but the values "
                "below are given as defaults here. The color space is declared "
                "in a color space information tag, with the default "
                "being the value that gives the optimal image characteristics "
                "Interoperability these conditions.",
                ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x02bc, "XMLPacket", "XML Packet",
                "XMP Metadata (Adobe technote 9-14-02)",
                ifd0Id, otherTags, unsignedByte, printValue),
        TagInfo(0x828d, "CFARepeatPatternDim", "CFARepeatPatternDim",
                "Contains two values representing the minimum rows and columns "
                "to define the repeating patterns of the color filter array",
                ifd0Id, otherTags, unsignedShort, printValue), // TIFF/EP Tag
        TagInfo(0x828e, "CFAPattern", "CFA Pattern", 
                "Indicates the color filter array (CFA) geometric pattern of the image "
                "sensor when a one-chip color area sensor is used. It does not apply to "
                "all sensing methods", 
                ifd0Id, otherTags, unsignedByte, printValue), // TIFF/EP Tag
        TagInfo(0x828f, "BatteryLevel", "Battery Level",
                "Contains a value of the battery level as a fraction or string",
                ifd0Id, otherTags, unsignedRational, printValue), // TIFF/EP Tag
        TagInfo(0x83bb, "IPTCNAA", "IPTC/NAA",
                "Contains an IPTC/NAA record",
                ifd0Id, otherTags, unsignedLong, printValue), // TIFF/EP Tag
        TagInfo(0x8298, "Copyright", "Copyright",
                "Copyright information. In this standard the tag is used to "
                "indicate both the photographer and editor copyrights. It is "
                "the copyright notice of the person or organization claiming "
                "rights to the image. The Interoperability copyright "
                "statement including date and rights should be written in this "
                "field; e.g., \"Copyright, John Smith, 19xx. All rights "
                "reserved.\". In this standard the field records both the "
                "photographer and editor copyrights, with each recorded in a "
                "separate part of the statement. When there is a clear "
                "distinction between the photographer and editor copyrights, "
                "these are to be written in the order of photographer followed "
                "by editor copyright, separated by NULL (in this case, "
                "since the statement also ends with a NULL, there are two NULL "
                "codes) (see example 1). When only the photographer is given, "
                "it is terminated by one NULL code (see example 2). When only "
                "the editor copyright is given, "
                "the photographer copyright part consists of one space followed "
                "by a terminating NULL code, then the editor copyright is given "
                "(see example 3). When the field is left blank, it is treated as unknown.",
                ifd0Id, otherTags, asciiString, print0x8298),
        TagInfo(0x8649, "ImageResources", "Image Resources Block",
                "Contains information embedded by the Adobe Photoshop application",
                ifd0Id, otherTags, undefined, printValue),
        TagInfo(0x8769, "ExifTag", "ExifIFDPointer",
                "A pointer to the Exif IFD. Interoperability, Exif IFD has the "
                "same structure as that of the IFD specified in TIFF. "
                "ordinarily, however, it does not contain image data as in "
                "the case of TIFF.",
                ifd0Id, exifFormat, unsignedLong, printValue),
        TagInfo(0x8773, "InterColorProfile", "InterColorProfile",
                "Contains an InterColor Consortium (ICC) format color space characterization/profile",
                ifd0Id, otherTags, undefined, printValue),
        TagInfo(0x8825, "GPSTag", "GPSInfoIFDPointer",
                "A pointer to the GPS Info IFD. The "
                "Interoperability structure of the GPS Info IFD, like that of "
                "Exif IFD, has no image data.",
                ifd0Id, exifFormat, unsignedLong, printValue),
        TagInfo(0x9216, "TIFFEPStandardID", "TIFF/EP Standard ID", 
                "Contains four ASCII characters representing the TIFF/EP standard "
                "version of a TIFF/EP file, eg '1', '0', '0', '0'", 
                ifd0Id, otherTags, unsignedByte, printValue), // TIFF/EP Tag
        // End of list marker
        TagInfo(0xffff, "(UnknownIfdTag)", "Unknown IFD tag",
                "Unknown IFD tag",
                ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    //! ExposureProgram, tag 0x8822
    extern const TagDetails exifExposureProgram[] = {
        { 0, "Not defined"       },
        { 1, "Manual"            },
        { 2, "Auto"              },
        { 3, "Aperture priority" },
        { 4, "Shutter priority"  },
        { 5, "Creative program"  },
        { 6, "Action program"    },
        { 7, "Portrait mode"     },
        { 8, "Landscape mode"    }
    };

    //! MeteringMode, tag 0x9207
    extern const TagDetails exifMeteringMode[] = {
        { 0,   "Unknown"                 },
        { 1,   "Average"                 },
        { 2,   "Center weighted average" },
        { 3,   "Spot"                    },
        { 4,   "Multi-spot"              },
        { 5,   "Multi-segment"           },
        { 6,   "Partial"                 },
        { 255, "Other"                   }
    };

    //! LightSource, tag 0x9208
    extern const TagDetails exifLightSource[] = {
        {   0, "Unknown"                                 },
        {   1, "Daylight"                                },
        {   2, "Fluorescent"                             },
        {   3, "Tungsten (incandescent light)"           },
        {   4, "Flash"                                   },
        {   9, "Fine weather"                            },
        {  10, "Cloudy weather"                          },
        {  11, "Shade"                                   },
        {  12, "Daylight fluorescent (D 5700 - 7100K)"   },
        {  13, "Day white fluorescent (N 4600 - 5400K)"  },
        {  14, "Cool white fluorescent (W 3900 - 4500K)" },
        {  15, "White fluorescent (WW 3200 - 3700K)"     },
        {  17, "Standard light A"                        },
        {  18, "Standard light B"                        },
        {  19, "Standard light C"                        },
        {  20, "D55"                                     },
        {  21, "D65"                                     },
        {  22, "D75"                                     },
        {  23, "D50"                                     },
        {  24, "ISO studio tungsten"                     },
        { 255, "Other light source"                      }
    };

    //! Flash, tag 0x9209
    extern const TagDetails exifFlash[] = {
        { 0x00, "No flash"                                                      },
        { 0x01, "Fired"                                                         },
        { 0x05, "Fired, strobe return light not detected"                       },
        { 0x07, "Fired, strobe return light detected"                           },
        { 0x09, "Yes, compulsory"                                               },
        { 0x0d, "Yes, compulsory, return light not detected"                    },
        { 0x0f, "Yes, compulsory, return light detected"                        },
        { 0x10, "No, compulsory"                                                },
        { 0x18, "No, auto"                                                      },
        { 0x19, "Yes, auto"                                                     },
        { 0x1d, "Yes, auto, return light not detected"                          },
        { 0x1f, "Yes, auto, return light detected"                              },
        { 0x20, "No flash function"                                             },
        { 0x41, "Yes, red-eye reduction"                                        },
        { 0x45, "Yes, red-eye reduction, return light not detected"             },
        { 0x47, "Yes, red-eye reduction, return light detected"                 },
        { 0x49, "Yes, compulsory, red-eye reduction"                            },
        { 0x4d, "Yes, compulsory, red-eye reduction, return light not detected" },
        { 0x4f, "Yes, compulsory, red-eye reduction, return light detected"     },
        { 0x59, "Yes, auto, red-eye reduction"                                  },
        { 0x5d, "Yes, auto, red-eye reduction, return light not detected"       },
        { 0x5f, "Yes, auto, red-eye reduction, return light detected"           }
    };

    //! ColorSpace, tag 0xa001
    extern const TagDetails exifColorSpace[] = {
        {      1, "sRGB"         },
        {      2, "Adobe RGB"    },    // Not defined to Exif 2.2 spec. But used by a lot of cameras.
        { 0xffff, "Uncalibrated" }
    };

    //! SensingMethod, tag 0xa217
    extern const TagDetails exifSensingMethod[] = {
        { 1, "Not defined"             },
        { 2, "One-chip color area"     },
        { 3, "Two-chip color area"     },
        { 4, "Three-chip color area"   },
        { 5, "Color sequential area"   },
        { 7, "Trilinear sensor"        },
        { 8, "Color sequential linear" }
    };

    //! FileSource, tag 0xa300
    extern const TagDetails exifFileSource[] = {
        { 1, "Film scanner"            },	// Not defined to Exif 2.2 spec. 
        { 2, "Reflexion print scanner" },	// but used by some scanner device softwares.
        { 3, "Digital still camera"    }
    };

    //! SceneType, tag 0xa301
    extern const TagDetails exifSceneType[] = {
        { 1, "Directly photographed" }
    };

    //! exifCustomRendered, tag 0xa401
    extern const TagDetails exifCustomRendered[] = {
        { 0, "Normal process" },
        { 1, "Custom process" }
    };

    //! ExposureMode, tag 0xa402
    extern const TagDetails exifExposureMode[] = {
        { 0, "Auto"         },
        { 1, "Manual"       },
        { 2, "Auto bracket" }
    };

    //! WhiteBalance, tag 0xa403
    extern const TagDetails exifWhiteBalance[] = {
        { 0, "Auto"   },
        { 1, "Manual" }
    };

    //! SceneCaptureType, tag 0xa406
    extern const TagDetails exifSceneCaptureType[] = {
        { 0, "Standard"    },
        { 1, "Landscape"   },
        { 2, "Portrait"    },
        { 3, "Night scene" }
    };

    //! GainControl, tag 0xa407
    extern const TagDetails exifGainControl[] = {
        { 0, "None"           },
        { 1, "Low gain up"    },
        { 2, "High gain up"   },
        { 3, "Low gain down"  },
        { 4, "High gain down" }
    };

    //! Contrast, tag 0xa408
    extern const TagDetails exifContrast[] = {
        { 0, "Normal" },
        { 1, "Soft"   },
        { 2, "Hard"   }
    };

    //! Saturation, tag 0xa409
    extern const TagDetails exifSaturation[] = {
        { 0, "Normal" },
        { 1, "Low"    },
        { 2, "High"   }
    };

    //! Sharpness, tag 0xa40a
    extern const TagDetails exifSharpness[] = {
        { 0, "Normal" },
        { 1, "Soft"   },
        { 2, "Hard"   }
    };

    //! SubjectDistanceRange, tag 0xa40c
    extern const TagDetails exifSubjectDistanceRange[] = {
        { 0, "Unknown"      },
        { 1, "Macro"        },
        { 2, "Close view"   },
        { 3, "Distant view" }
    };

    // Exif IFD Tags
    static const TagInfo exifTagInfo[] = {
        TagInfo(0x829a, "ExposureTime", "Exposure Time", "Exposure time", exifIfdId, captureCond, unsignedRational, print0x829a),
        TagInfo(0x829d, "FNumber", "FNumber", "F number", exifIfdId, captureCond, unsignedRational, print0x829d),
        TagInfo(0x8822, "ExposureProgram", "ExposureProgram", "Exposure program", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifExposureProgram)),
        TagInfo(0x8824, "SpectralSensitivity", "Spectral Sensitivity", "Spectral sensitivity", exifIfdId, captureCond, asciiString, printValue),
        TagInfo(0x8827, "ISOSpeedRatings", "ISO Speed Ratings", "ISO speed ratings", exifIfdId, captureCond, unsignedShort, print0x8827),
        TagInfo(0x8828, "OECF", "OECF", "Optoelectric coefficient", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0x9000, "ExifVersion", "Exif Version", "Exif Version", exifIfdId, exifVersion, undefined, printValue),
        TagInfo(0x9003, "DateTimeOriginal", "Date and Time (original)", "Date and time original image was generated", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9004, "DateTimeDigitized", "Date and Time (digitized)", "Date and time image was made digital data", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9101, "ComponentsConfiguration", "ComponentsConfiguration", "Meaning of each component", exifIfdId, imgConfig, undefined, print0x9101),
        TagInfo(0x9102, "CompressedBitsPerPixel", "Compressed Bits per Pixel", "Image compression mode", exifIfdId, imgConfig, unsignedRational, printFloat),
        TagInfo(0x9201, "ShutterSpeedValue", "Shutter speed", "Shutter speed", exifIfdId, captureCond, signedRational, print0x9201),
        TagInfo(0x9202, "ApertureValue", "Aperture", "Aperture", exifIfdId, captureCond, unsignedRational, print0x9202),
        TagInfo(0x9203, "BrightnessValue", "Brightness", "Brightness", exifIfdId, captureCond, signedRational, printFloat),
        TagInfo(0x9204, "ExposureBiasValue", "Exposure Bias", "Exposure bias", exifIfdId, captureCond, signedRational, print0x9204),
        TagInfo(0x9205, "MaxApertureValue", "MaxApertureValue", "Maximum lens aperture", exifIfdId, captureCond, unsignedRational, print0x9202),
        TagInfo(0x9206, "SubjectDistance", "Subject Distance", "Subject distance", exifIfdId, captureCond, unsignedRational, print0x9206),
        TagInfo(0x9207, "MeteringMode", "Metering Mode", "Metering mode", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifMeteringMode)),
        TagInfo(0x9208, "LightSource", "Light Source", "Light source", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifLightSource)),
        TagInfo(0x9209, "Flash", "Flash", "Flash", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifFlash)),
        TagInfo(0x920a, "FocalLength", "Focal Length", "Lens focal length", exifIfdId, captureCond, unsignedRational, print0x920a),
        TagInfo(0x9214, "SubjectArea", "Subject Area", "Subject area", exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0x927c, "MakerNote", "Maker Note", "Manufacturer notes", exifIfdId, userInfo, undefined, printValue),
        TagInfo(0x9286, "UserComment", "User Comment", "User comments", exifIfdId, userInfo, comment, print0x9286),
        TagInfo(0x9290, "SubSecTime", "SubsecTime", "DateTime subseconds", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9291, "SubSecTimeOriginal", "SubSecTimeOriginal", "DateTimeOriginal subseconds", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9292, "SubSecTimeDigitized", "SubSecTimeDigitized", "DateTimeDigitized subseconds", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0xa000, "FlashpixVersion", "FlashPixVersion", "Supported Flashpix version", exifIfdId, exifVersion, undefined, printValue),
        TagInfo(0xa001, "ColorSpace", "Color Space", "Color space information", exifIfdId, imgCharacter, unsignedShort, EXV_PRINT_TAG(exifColorSpace)),
        TagInfo(0xa002, "PixelXDimension", "PixelXDimension", "Valid image width", exifIfdId, imgConfig, unsignedLong, printValue),
        TagInfo(0xa003, "PixelYDimension", "PixelYDimension", "Valid image height", exifIfdId, imgConfig, unsignedLong, printValue),
        TagInfo(0xa004, "RelatedSoundFile", "RelatedSoundFile", "Related audio file", exifIfdId, relatedFile, asciiString, printValue),
        TagInfo(0xa005, "InteroperabilityTag", "InteroperabilityIFDPointer", "Interoperability IFD Pointer", exifIfdId, exifFormat, unsignedLong, printValue),
        TagInfo(0xa20b, "FlashEnergy", "Flash Energy", "Flash energy", exifIfdId, captureCond, unsignedRational, printValue),
        TagInfo(0xa20c, "SpatialFrequencyResponse", "Spatial Frequency Response", "Spatial frequency response", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa20e, "FocalPlaneXResolution", "Focal Plane x-Resolution", "Focal plane X resolution", exifIfdId, captureCond, unsignedRational, printFloat),
        TagInfo(0xa20f, "FocalPlaneYResolution", "Focal Plane y-Resolution", "Focal plane Y resolution", exifIfdId, captureCond, unsignedRational, printFloat),
        TagInfo(0xa210, "FocalPlaneResolutionUnit", "Focal Plane Resolution Unit", "Focal plane resolution unit", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifUnit)),
        TagInfo(0xa214, "SubjectLocation", "Subject Location", "Subject location", exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0xa215, "ExposureIndex", "Exposure index", "Exposure index", exifIfdId, captureCond, unsignedRational, printValue),
        TagInfo(0xa217, "SensingMethod", "Sensing Method", "Sensing method", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSensingMethod)),
        TagInfo(0xa300, "FileSource", "File Source", "File source", exifIfdId, captureCond, undefined, EXV_PRINT_TAG(exifFileSource)),
        TagInfo(0xa301, "SceneType", "Scene Type", "Scene type", exifIfdId, captureCond, undefined, EXV_PRINT_TAG(exifSceneType)),
        TagInfo(0xa302, "CFAPattern", "CFA Pattern", "CFA pattern", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa401, "CustomRendered", "Custom Rendered", "Custom image processing", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifCustomRendered)),
        TagInfo(0xa402, "ExposureMode", "Exposure Mode", "Exposure mode", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifExposureMode)),
        TagInfo(0xa403, "WhiteBalance", "White Balance", "White balance", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifWhiteBalance)),
        TagInfo(0xa404, "DigitalZoomRatio", "Digital Zoom Ratio", "Digital zoom ratio", exifIfdId, captureCond, unsignedRational, print0xa404),
        TagInfo(0xa405, "FocalLengthIn35mmFilm", "Focal Length In 35mm Film", "Focal length in 35 mm film", exifIfdId, captureCond, unsignedShort, print0xa405),
        TagInfo(0xa406, "SceneCaptureType", "Scene Capture Type", "Scene capture type", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSceneCaptureType)),
        TagInfo(0xa407, "GainControl", "Gain Control", "Gain control", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifGainControl)),
        TagInfo(0xa408, "Contrast", "Contrast", "Contrast", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifContrast)),
        TagInfo(0xa409, "Saturation", "Saturation", "Saturation", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSaturation)),
        TagInfo(0xa40a, "Sharpness", "Sharpness", "Sharpness", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSharpness)),
        TagInfo(0xa40b, "DeviceSettingDescription", "Device Setting Description", "Device settings description", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa40c, "SubjectDistanceRange", "Subject Distance Range", "Subject distance range", exifIfdId, captureCond, unsignedShort, EXV_PRINT_TAG(exifSubjectDistanceRange)),
        TagInfo(0xa420, "ImageUniqueID", "Image Unique ID", "Unique image ID", exifIfdId, otherTags, asciiString, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownExifTag)", "Unknown Exif tag", "Unknown Exif tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    //! GPS latitude reference, tag 0x0001; also GPSDestLatitudeRef, tag 0x0013
    extern const TagDetails exifGPSLatitudeRef[] = {
        { 78, "North" },
        { 83, "South" }
    };

    //! GPS longitude reference, tag 0x0003; also GPSDestLongitudeRef, tag 0x0015
    extern const TagDetails exifGPSLongitudeRef[] = {
        { 69, "East" },
        { 87, "West" }
    };

    //! GPS altitude reference, tag 0x0005
    extern const TagDetails exifGPSAltitudeRef[] = {
        { 0, "Above sea level" },
        { 1, "Below sea level" }
    };

    //! GPS speed reference, tag 0x000c
    extern const TagDetails exifGPSSpeedRef[] = {
        { 75, "km/h" },
        { 77, "mph" },
        { 78, "knots" }
    };

    // GPS Info Tags
    static const TagInfo gpsTagInfo[] = {
        TagInfo(0x0000, "GPSVersionID", "GPSVersionID", "GPS tag version", gpsIfdId, gpsTags, unsignedByte, printValue),
        TagInfo(0x0001, "GPSLatitudeRef", "GPSLatitudeRef", "North or South Latitude", gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLatitudeRef)),
        TagInfo(0x0002, "GPSLatitude", "GPSLatitude", "Latitude", gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0003, "GPSLongitudeRef", "GPSLongitudeRef", "East or West Longitude", gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLongitudeRef)),
        TagInfo(0x0004, "GPSLongitude", "GPSLongitude", "Longitude", gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0005, "GPSAltitudeRef", "GPSAltitudeRef", "Altitude reference", gpsIfdId, gpsTags, unsignedByte, EXV_PRINT_TAG(exifGPSAltitudeRef)),
        TagInfo(0x0006, "GPSAltitude", "GPSAltitude", "Altitude", gpsIfdId, gpsTags, unsignedRational, print0x0006),
        TagInfo(0x0007, "GPSTimeStamp", "GPSTimeStamp", "GPS time (atomic clock)", gpsIfdId, gpsTags, unsignedRational, print0x0007),
        TagInfo(0x0008, "GPSSatellites", "GPSSatellites", "GPS satellites used for measurement", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0009, "GPSStatus", "GPSStatus", "GPS receiver status", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000a, "GPSMeasureMode", "GPSMeasureMode", "GPS measurement mode", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000b, "GPSDOP", "GPSDOP", "Measurement precision", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x000c, "GPSSpeedRef", "GPSSpeedRef", "Speed unit", gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSSpeedRef)),
        TagInfo(0x000d, "GPSSpeed", "GPSSpeed", "Speed of GPS receiver", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x000e, "GPSTrackRef", "GPSTrackRef", "Reference for direction of movement", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000f, "GPSTrack", "GPSTrack", "Direction of movement", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0010, "GPSImgDirectionRef", "GPSImgDirectionRef", "Reference for direction of image", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0011, "GPSImgDirection", "GPSImgDirection", "Direction of image", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0012, "GPSMapDatum", "GPSMapDatum", "Geodetic survey data used", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0013, "GPSDestLatitudeRef", "GPSDestLatitudeRef", "Reference for latitude of destination", gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLatitudeRef)),
        TagInfo(0x0014, "GPSDestLatitude", "GPSDestLatitude", "Latitude of destination", gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0015, "GPSDestLongitudeRef", "GPSDestLongitudeRef", "Reference for longitude of destination", gpsIfdId, gpsTags, asciiString, EXV_PRINT_TAG(exifGPSLongitudeRef)),
        TagInfo(0x0016, "GPSDestLongitude", "GPSDestLongitude", "Longitude of destination", gpsIfdId, gpsTags, unsignedRational, printDegrees),
        TagInfo(0x0017, "GPSDestBearingRef", "GPSDestBearingRef", "Reference for bearing of destination", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0018, "GPSDestBearing", "GPSDestBearing", "Bearing of destination", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0019, "GPSDestDistanceRef", "GPSDestDistanceRef", "Reference for distance to destination", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x001a, "GPSDestDistance", "GPSDestDistance", "Distance to destination", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x001b, "GPSProcessingMethod", "GPSProcessingMethod", "Name of GPS processing method", gpsIfdId, gpsTags, undefined, printValue),
        TagInfo(0x001c, "GPSAreaInformation", "GPSAreaInformation", "Name of GPS area", gpsIfdId, gpsTags, undefined, printValue),
        TagInfo(0x001d, "GPSDateStamp", "GPSDateStamp", "GPS date", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x001e, "GPSDifferential", "GPSDifferential", "GPS differential correction", gpsIfdId, gpsTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownGpsTag)", "Unknown GPSInfo tag", "Unknown GPSInfo tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    // Exif Interoperability IFD Tags
    static const TagInfo iopTagInfo[] = {
        TagInfo(0x0001, "InteroperabilityIndex", "InteroperabilityIndex", 
                "Indicates the identification of the Interoperability rule. "
                "Use \"R98\" for stating ExifR98 Rules. Four bytes used "
                "including the termination code (NULL). see the separate "
                "volume of Recommended Exif Interoperability Rules (ExifR98) "
                "for other tags used for ExifR98.",
                iopIfdId, iopTags, asciiString, printValue),
        TagInfo(0x0002, "InteroperabilityVersion", "InteroperabilityVersion", "Interoperability version", iopIfdId, iopTags, undefined, printValue),
        TagInfo(0x1000, "RelatedImageFileFormat", "RelatedImageFileFormat",
                "File format of image file",
                iopIfdId, iopTags, asciiString, printValue),
        TagInfo(0x1001, "RelatedImageWidth", "RelatedImageWidth",
                "Image width",
                iopIfdId, iopTags, unsignedLong, printValue),
        TagInfo(0x1002, "RelatedImageLength", "RelatedImageLength",
                "Image height",
                iopIfdId, iopTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownIopTag)", "Unknown Exif Interoperability tag", "Unknown Exif Interoperability tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    // Unknown Tag
    static const TagInfo unknownTag(0xffff, "Unknown tag", "Unknown tag", "Unknown tag", ifdIdNotSet, sectionIdNotSet, asciiString, printValue);

    // Tag lookup lists with tag names, desc and where they (preferably) belong to;
    // this is an array with pointers to one list per IFD. The IfdId is used as the
    // index into the array.
    const TagInfo* ExifTags::tagInfos_[] = {
        0,
        ifdTagInfo, exifTagInfo, gpsTagInfo, iopTagInfo, ifdTagInfo,
        0
    };

    // Lookup list for registered makernote tag info tables
    const TagInfo* ExifTags::makerTagInfos_[];

    // All makernote ifd ids, in the same order as the tag infos in makerTagInfos_
    IfdId ExifTags::makerIfdIds_[];

    void ExifTags::registerBaseTagInfo(IfdId ifdId)
    {
        registerMakerTagInfo(ifdId, ifdTagInfo);
    }

    void ExifTags::registerMakerTagInfo(IfdId ifdId, const TagInfo* tagInfo)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS; ++i) {
            if (makerIfdIds_[i] == 0) {
                makerIfdIds_[i] = ifdId;
                makerTagInfos_[i] = tagInfo;
                break;
            }
        }
        if (i == MAX_MAKER_TAG_INFOS) throw Error(16);
    } // ExifTags::registerMakerTagInfo

    int ExifTags::tagInfoIdx(uint16_t tag, IfdId ifdId)
    {
        const TagInfo* tagInfo = tagInfos_[ifdId];
        if (tagInfo == 0) return -1;
        int idx;
        for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
            if (tagInfo[idx].tag_ == tag) return idx;
        }
        return -1;
    } // ExifTags::tagInfoIdx

    const TagInfo* ExifTags::makerTagInfo(uint16_t tag, IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i == MAX_MAKER_TAG_INFOS) return 0;

        for (int k = 0; makerTagInfos_[i][k].tag_ != 0xffff; ++k) {
            if (makerTagInfos_[i][k].tag_ == tag) return &makerTagInfos_[i][k];
        }

        return 0;
    } // ExifTags::makerTagInfo

    const TagInfo* ExifTags::makerTagInfo(const std::string& tagName,
                                          IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i == MAX_MAKER_TAG_INFOS) return 0;

        for (int k = 0; makerTagInfos_[i][k].tag_ != 0xffff; ++k) {
            if (makerTagInfos_[i][k].name_ == tagName) {
                return &makerTagInfos_[i][k];
            }
        }

        return 0;
    } // ExifTags::makerTagInfo

    bool ExifTags::isMakerIfd(IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        return i != MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != IfdId(0);
    }

    std::string ExifTags::tagName(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) return tagInfos_[ifdId][idx].name_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->name_;
        }
        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << tag;
        return os.str();
    } // ExifTags::tagName

    const char* ExifTags::tagTitle(uint16_t tag, IfdId ifdId)
    {
        return tagLabel(tag, ifdId);
    } // ExifTags::tagTitle

    const char* ExifTags::tagLabel(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return unknownTag.title_;
            return tagInfos_[ifdId][idx].title_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->title_;
        }
        return "";
    } // ExifTags::tagLabel

    const char* ExifTags::tagDesc(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return unknownTag.desc_;
            return tagInfos_[ifdId][idx].desc_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->desc_;
        }
        return "";
    } // ExifTags::tagDesc

    const char* ExifTags::sectionName(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return sectionInfo_[unknownTag.sectionId_].name_;
            const TagInfo* tagInfo = tagInfos_[ifdId];
            return sectionInfo_[tagInfo[idx].sectionId_].name_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return sectionInfo_[tagInfo->sectionId_].name_;
        }
        return "";
    } // ExifTags::sectionName

    const char* ExifTags::sectionDesc(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return sectionInfo_[unknownTag.sectionId_].desc_;
            const TagInfo* tagInfo = tagInfos_[ifdId];
            return sectionInfo_[tagInfo[idx].sectionId_].desc_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return sectionInfo_[tagInfo->sectionId_].desc_;
        }
        return "";
    } // ExifTags::sectionDesc

    uint16_t ExifTags::tag(const std::string& tagName, IfdId ifdId)
    {
        uint16_t tag = 0xffff;
        if (isExifIfd(ifdId)) {
            const TagInfo* tagInfo = tagInfos_[ifdId];
            if (tagInfo) {
                int idx;
                for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
                    if (tagInfo[idx].name_ == tagName) break;
                }
                tag = tagInfo[idx].tag_;
            }
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tagName, ifdId);
            if (tagInfo != 0) tag = tagInfo->tag_;
        }
        if (tag == 0xffff) {
            if (!isHex(tagName, 4, "0x")) throw Error(7, tagName, ifdId);
            std::istringstream is(tagName);
            is >> std::hex >> tag;
        }
        return tag;
    } // ExifTags::tag

    IfdId ExifTags::ifdIdByIfdItem(const std::string& ifdItem)
    {
        int i;
        for (i = int(lastIfdId) - 1; i > 0; --i) {
            if (ifdInfo_[i].item_ == ifdItem) break;
        }
        return IfdId(i);
    }

    const char* ExifTags::ifdName(IfdId ifdId)
    {
        return ifdInfo_[ifdId].name_;
    }

    const char* ExifTags::ifdItem(IfdId ifdId)
    {
        return ifdInfo_[ifdId].item_;
    }

    const char* ExifTags::sectionName(SectionId sectionId)
    {
        return sectionInfo_[sectionId].name_;
    }

    SectionId ExifTags::sectionId(const std::string& sectionName)
    {
        int i;
        for (i = int(lastSectionId) - 1; i > 0; --i) {
            if (sectionInfo_[i].name_ == sectionName) break;
        }
        return SectionId(i);
    }

    TypeId ExifTags::tagType(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) return tagInfos_[ifdId][idx].typeId_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->typeId_;
        }
        return unknownTag.typeId_;
    }

    std::ostream& ExifTags::printTag(std::ostream& os,
                                     uint16_t tag,
                                     IfdId ifdId,
                                     const Value& value)
    {
        if (value.count() == 0) return os;
        PrintFct fct = printValue;
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) {
                fct = tagInfos_[ifdId][idx].printFct_;
            }
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) fct = tagInfo->printFct_;
        }
        return fct(os, value);
    } // ExifTags::printTag

    void ExifTags::taglist(std::ostream& os)
    {
        for (int i=0; ifdTagInfo[i].tag_ != 0xffff; ++i) {
            os << ifdTagInfo[i] << "\n";
        }
        for (int i=0; exifTagInfo[i].tag_ != 0xffff; ++i) {
            os << exifTagInfo[i] << "\n";
        }
        for (int i=0; iopTagInfo[i].tag_ != 0xffff; ++i) {
            os << iopTagInfo[i] << "\n";
        }
        for (int i=0; gpsTagInfo[i].tag_ != 0xffff; ++i) {
            os << gpsTagInfo[i] << "\n";
        }
    } // ExifTags::taglist

    void ExifTags::makerTaglist(std::ostream& os, IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i != MAX_MAKER_TAG_INFOS) {
            const TagInfo* mnTagInfo = makerTagInfos_[i];
            for (int k=0; mnTagInfo[k].tag_ != 0xffff; ++k) {
                os << mnTagInfo[k] << "\n";
            }
        }
    } // ExifTags::makerTaglist

    const char* ExifKey::familyName_ = "Exif";

    ExifKey::ExifKey(const std::string& key)
        : tag_(0), ifdId_(ifdIdNotSet), ifdItem_(""),
          idx_(0), key_(key)
    {
        decomposeKey();
    }

    ExifKey::ExifKey(uint16_t tag, const std::string& ifdItem)
        : tag_(0), ifdId_(ifdIdNotSet), ifdItem_(""),
          idx_(0), key_("")
    {
        IfdId ifdId = ExifTags::ifdIdByIfdItem(ifdItem);
        if (ExifTags::isMakerIfd(ifdId)) {
            MakerNote::AutoPtr makerNote = MakerNoteFactory::create(ifdId);
            if (makerNote.get() == 0) throw Error(23, ifdId);
        }
        tag_ = tag;
        ifdId_ = ifdId;
        ifdItem_ = ifdItem;
        makeKey();
    }

    ExifKey::ExifKey(const Entry& e)
        : tag_(e.tag()), ifdId_(e.ifdId()),
          ifdItem_(ExifTags::ifdItem(e.ifdId())),
          idx_(e.idx()), key_("")
    {
        makeKey();
    }

    ExifKey::ExifKey(const ExifKey& rhs)
        : Key(rhs), tag_(rhs.tag_), ifdId_(rhs.ifdId_), ifdItem_(rhs.ifdItem_),
          idx_(rhs.idx_), key_(rhs.key_)
    {
    }

    ExifKey::~ExifKey()
    {
    }

    ExifKey& ExifKey::operator=(const ExifKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        tag_ = rhs.tag_;
        ifdId_ = rhs.ifdId_;
        ifdItem_ = rhs.ifdItem_;
        idx_ = rhs.idx_;
        key_ = rhs.key_;
        return *this;
    }

    std::string ExifKey::tagName() const
    {
        return ExifTags::tagName(tag_, ifdId_);
    }

    std::string ExifKey::tagLabel() const 	
    {
        return ExifTags::tagLabel(tag_, ifdId_);
    }

    ExifKey::AutoPtr ExifKey::clone() const
    {
        return AutoPtr(clone_());
    }

    ExifKey* ExifKey::clone_() const
    {
        return new ExifKey(*this);
    }

    std::string ExifKey::sectionName() const
    {
        return ExifTags::sectionName(tag(), ifdId());
    }

    void ExifKey::decomposeKey()
    {
        // Get the family name, IFD name and tag name parts of the key
        std::string::size_type pos1 = key_.find('.');
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string familyName = key_.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error(6, key_);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key_.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string ifdItem = key_.substr(pos0, pos1 - pos0);
        if (ifdItem == "") throw Error(6, key_);
        std::string tagName = key_.substr(pos1 + 1);
        if (tagName == "") throw Error(6, key_);

        // Find IfdId
        IfdId ifdId = ExifTags::ifdIdByIfdItem(ifdItem);
        if (ifdId == ifdIdNotSet) throw Error(6, key_);
        if (ExifTags::isMakerIfd(ifdId)) {
            MakerNote::AutoPtr makerNote = MakerNoteFactory::create(ifdId);
            if (makerNote.get() == 0) throw Error(6, key_);
        }
        // Convert tag
        uint16_t tag = ExifTags::tag(tagName, ifdId);

        // Translate hex tag name (0xabcd) to a real tag name if there is one
        tagName = ExifTags::tagName(tag, ifdId);

        tag_ = tag;
        ifdId_ = ifdId;
        ifdItem_ = ifdItem;
        key_ = familyName + "." + ifdItem + "." + tagName;
    }

    void ExifKey::makeKey()
    {
        key_ =   std::string(familyName_)
               + "." + ifdItem_
               + "." + ExifTags::tagName(tag_, ifdId_);
    }

    // *************************************************************************
    // free functions

    bool isExifIfd(IfdId ifdId)
    {
        bool rc;
        switch (ifdId) {
        case ifd0Id:    rc = true; break;
        case exifIfdId: rc = true; break;
        case gpsIfdId:  rc = true; break;
        case iopIfdId:  rc = true; break;
        case ifd1Id:    rc = true; break;
        default:        rc = false; break;
        }
        return rc;
    } // isExifIfd

    std::ostream& operator<<(std::ostream& os, const TagInfo& ti)
    {
        ExifKey exifKey(ti.tag_, ExifTags::ifdItem(ti.ifdId_));
        return os << ExifTags::tagName(ti.tag_, ti.ifdId_) << ",\t"
                  << std::dec << ti.tag_ << ",\t"
                  << "0x" << std::setw(4) << std::setfill('0')
                  << std::right << std::hex << ti.tag_ << ",\t"
                  << ExifTags::ifdName(ti.ifdId_) << ",\t"
                  << exifKey.key() << ",\t"
                  << TypeInfo::typeName(
                      ExifTags::tagType(ti.tag_, ti.ifdId_)) << ",\t"
                  << ExifTags::tagDesc(ti.tag_, ti.ifdId_);
    }

    std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, Rational& r)
    {
        int32_t nominator;
        int32_t denominator;
        char c;
        is >> nominator >> c >> denominator;
        if (is && c == '/') r = std::make_pair(nominator, denominator);
        return is;
    }

    std::ostream& operator<<(std::ostream& os, const URational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, URational& r)
    {
        uint32_t nominator;
        uint32_t denominator;
        char c;
        is >> nominator >> c >> denominator;
        if (is && c == '/') r = std::make_pair(nominator, denominator);
        return is;
    }

    std::ostream& printValue(std::ostream& os, const Value& value)
    {
        return os << value;
    }

    std::ostream& printLong(std::ostream& os, const Value& value)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<long>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printLong

    std::ostream& printFloat(std::ostream& os, const Value& value)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<float>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printFloat

    std::ostream& printDegrees(std::ostream& os, const Value& value)
    {
        if (value.count() == 3) {
            std::ostringstream oss;
            oss.copyfmt(os);
            static const char unit[4] = "'\"";
            int n;
            for (n = 2; n > 0; --n) {
                if (value.toRational(n).first != 0) break;
            }
            for (int i = 0; i < n + 1; ++i) {
                const int32_t d = value.toRational(i).second;
                const int p = d > 1 ? d > 19 ? d > 199 ? d > 1999 ? 4 : 3 : 2 : 1 : 0;
                os << std::fixed << std::setprecision(p) << value.toFloat(i)
                   << unit[i] << " ";
            }
            os.copyfmt(oss);
        }
        else {
            os << value;
        }

        return os;
    } // printDegrees
    
    std::ostream& print0x0006(std::ostream& os, const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        const int32_t d = value.toRational().second;
        const int p = d > 1 ? 1 : 0;
        os << std::fixed << std::setprecision(p) << value.toFloat() << " m";
        os.copyfmt(oss);

        return os;
    }

    std::ostream& print0x0007(std::ostream& os, const Value& value)
    {
        if (value.count() == 3) {
            std::ostringstream oss;
            oss.copyfmt(os);
            const float sec = 3600 * value.toFloat(0) 
                              + 60 * value.toFloat(1) 
                              + value.toFloat(2);
            int p = 0;
            if (sec != static_cast<int>(sec)) p = 1;

            const int hh = static_cast<int>(sec / 3600);
            const int mm = static_cast<int>((sec - 3600 * hh) / 60);
            const float ss = sec - 3600 * hh - 60 * mm;

            os << std::setw(2) << std::setfill('0') << std::right << hh << ":" 
               << std::setw(2) << std::setfill('0') << std::right << mm << ":"
               << std::setw(2 + p * 2) << std::setfill('0') << std::right 
               << std::fixed << std::setprecision(p) << ss;

            os.copyfmt(oss);
        }
        else {
            os << value;
        }

        return os;
    }

    std::ostream& print0x8298(std::ostream& os, const Value& value)
    {
        // Print the copyright information in the format Photographer, Editor
        std::string val = value.toString();
        std::string::size_type pos = val.find('\0');
        if (pos != std::string::npos) {
            std::string photographer(val, 0, pos);
            if (photographer != " ") os << photographer;
            std::string editor(val, pos + 1);
            if (editor != "") {
                if (photographer != " ") os << ", ";
                os << editor;
            }
        }
        else {
            os << val;
        }
        return os;
    }

    std::ostream& print0x829a(std::ostream& os, const Value& value)
    {
        Rational t = value.toRational();
        if (t.first > 1 && t.second > 1 && t.second >= t.first) {
            t.second = static_cast<uint32_t>(
                static_cast<float>(t.second) / t.first + 0.5);
            t.first = 1;
        }
        if (t.second > 1 && t.second < t.first) {
            t.first = static_cast<uint32_t>(
                static_cast<float>(t.first) / t.second + 0.5);
            t.second = 1;
        }
        if (t.second == 1) {
            os << t.first << " s";
        }
        else {
            os << t.first << "/" << t.second << " s";
        }
        return os;
    }

    std::ostream& print0x829d(std::ostream& os, const Value& value)
    {
        Rational fnumber = value.toRational();
        if (fnumber.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << "F" << std::setprecision(2)
               << static_cast<float>(fnumber.first) / fnumber.second;
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& print0x8827(std::ostream& os, const Value& value)
    {
        return os << value.toLong();
    }

    std::ostream& print0x9101(std::ostream& os, const Value& value)
    {
        for (long i = 0; i < value.count(); ++i) {
            long l = value.toLong(i);
            switch (l) {
            case 0:  break;
            case 1:  os << "Y"; break;
            case 2:  os << "Cb"; break;
            case 3:  os << "Cr"; break;
            case 4:  os << "R"; break;
            case 5:  os << "G"; break;
            case 6:  os << "B"; break;
            default: os << "(" << l << ")"; break;
            }
        }
        return os;
    }

    std::ostream& print0x9201(std::ostream& os, const Value& value)
    {
        URational ur = exposureTime(value.toFloat());
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

    std::ostream& print0x9202(std::ostream& os, const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << "F" << std::setprecision(2) << fnumber(value.toFloat());
        os.copyfmt(oss);

        return os;
    }

    std::ostream& print0x9204(std::ostream& os, const Value& value)
    {
        Rational bias = value.toRational();
        if (bias.second <= 0) {
            os << "(" << bias.first << "/" << bias.second << ")";
        }
        else if (bias.first == 0) {
            os << "0";
        }
        else {
            int32_t d = gcd(bias.first, bias.second);
            int32_t num = std::abs(bias.first) / d;
            int32_t den = bias.second / d;
            os << (bias.first < 0 ? "-" : "+") << num;
            if (den != 1) {
                os << "/" << den;
            }
        }
        return os;
    }

    std::ostream& print0x9206(std::ostream& os, const Value& value)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << "Unknown";
        }
        else if (static_cast<uint32_t>(distance.first) == 0xffffffff) {
            os << "Infinity";
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

    std::ostream& print0x920a(std::ostream& os, const Value& value)
    {
        Rational length = value.toRational();
        if (length.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)length.first / length.second
               << " mm";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    // Todo: Implement this properly
    std::ostream& print0x9286(std::ostream& os, const Value& value)
    {
        if (value.size() > 8) {
            DataBuf buf(value.size());
            value.copy(buf.pData_, bigEndian);
            // Hack: Skip the leading 8-Byte character code, truncate
            // trailing '\0's and let the stream take care of the remainder
            std::string userComment(reinterpret_cast<char*>(buf.pData_) + 8, buf.size_ - 8);
            std::string::size_type pos = userComment.find_last_not_of('\0');
            os << userComment.substr(0, pos + 1);
        }
        return os;
    }

    std::ostream& print0xa404(std::ostream& os, const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.second == 0) {
            os << "Digital zoom not used";
        }
        else {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second;
            os.copyfmt(oss);
        }
        return os;
    }

    std::ostream& print0xa405(std::ostream& os, const Value& value)
    {
        long length = value.toLong();
        if (length == 0) {
            os << "Unknown";
        }
        else {
            os << length << ".0 mm";
        }
        return os;
    }

    float fnumber(float apertureValue)
    {
        return static_cast<float>(std::exp(std::log(2.0) * apertureValue / 2));
    }

    URational exposureTime(float shutterSpeedValue)
    {
        URational ur(1, 1);
        double tmp = std::exp(std::log(2.0) * shutterSpeedValue);
        if (tmp > 1) {
            ur.second = static_cast<long>(tmp + 0.5);
        }
        else {
            ur.first = static_cast<long>(1/tmp + 0.5);
        }
        return ur;
    }

}                                       // namespace Exiv2
