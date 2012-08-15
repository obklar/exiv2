//QuickTime Video - Detection And Display

#include "quicktimevideo.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include <math.h>

namespace Exiv2 {
    namespace Internal {

    extern const TagVocabulary qTimeFileType[] =  {
        {   "3g2a", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-0 V1.0" },
        {   "3g2b", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-A V1.0.0" },
        {   "3g2c", "3GPP2 Media (.3G2) compliant with 3GPP2 C.S0050-B v1.0" },
        {   "3ge6", "3GPP (.3GP) Release 6 MBMS Extended Presentations" },
        {   "3ge7", "3GPP (.3GP) Release 7 MBMS Extended Presentations" },
        {   "3gg6", "3GPP Release 6 General Profile" },
        {   "3gp1", "3GPP Media (.3GP) Release 1 (probably non-existent)" },
        {   "3gp2", "3GPP Media (.3GP) Release 2 (probably non-existent)" },
        {   "3gp3", "3GPP Media (.3GP) Release 3 (probably non-existent)" },
        {   "3gp4", "3GPP Media (.3GP) Release 4" },
        {   "3gp5", "3GPP Media (.3GP) Release 5" },
        {   "3gp6", "3GPP Media (.3GP) Release 6 Streaming Servers" },
        {   "3gs7", "3GPP Media (.3GP) Release 7 Streaming Servers" },
        {   "CAEP", "Canon Digital Camera" },
        {   "CDes", "Convergent Design" },
        {   "F4A ", "Audio for Adobe Flash Player 9+ (.F4A)" },
        {   "F4B ", "Audio Book for Adobe Flash Player 9+ (.F4B)" },
        {   "F4P ", "Protected Video for Adobe Flash Player 9+ (.F4P)" },
        {   "F4V ", "Video for Adobe Flash Player 9+ (.F4V)" },
        {   "JP2 ", "JPEG 2000 Image (.JP2) [ISO 15444-1 ?]" },
        {   "JP20", "Unknown, from GPAC samples (prob non-existent)" },
        {   "KDDI", "3GPP2 EZmovie for KDDI 3G cellphones" },
        {   "M4A ", "Apple iTunes AAC-LC (.M4A) Audio" },
        {   "M4B ", "Apple iTunes AAC-LC (.M4B) Audio Book" },
        {   "M4P ", "Apple iTunes AAC-LC (.M4P) AES Protected Audio" },
        {   "M4V ", "Apple iTunes Video (.M4V) Video" },
        {   "M4VH", "Apple TV (.M4V)" },
        {   "M4VP", "Apple iPhone (.M4V)" },
        {   "MPPI", "Photo Player, MAF [ISO/IEC 23000-3]" },
        {   "MSNV", "MPEG-4 (.MP4) for SonyPSP" },
        {   "NDAS", "MP4 v2 [ISO 14496-14] Nero Digital AAC Audio" },
        {   "NDSC", "MPEG-4 (.MP4) Nero Cinema Profile" },
        {   "NDSH", "MPEG-4 (.MP4) Nero HDTV Profile" },
        {   "NDSM", "MPEG-4 (.MP4) Nero Mobile Profile" },
        {   "NDSP", "MPEG-4 (.MP4) Nero Portable Profile" },
        {   "NDSS", "MPEG-4 (.MP4) Nero Standard Profile" },
        {   "NDXC", "H.264/MPEG-4 AVC (.MP4) Nero Cinema Profile" },
        {   "NDXH", "H.264/MPEG-4 AVC (.MP4) Nero HDTV Profile" },
        {   "NDXM", "H.264/MPEG-4 AVC (.MP4) Nero Mobile Profile" },
        {   "NDXP", "H.264/MPEG-4 AVC (.MP4) Nero Portable Profile" },
        {   "NDXS", "H.264/MPEG-4 AVC (.MP4) Nero Standard Profile" },
        {   "NIKO", "Nikon" },
        {   "ROSS", "Ross Video" },
        {   "avc1", "MP4 Base w/ AVC ext [ISO 14496-12:2005]" },
        {   "caqv", "Casio Digital Camera" },
        {   "da0a", "DMB MAF w/ MPEG Layer II aud, MOT slides, DLS, JPG/PNG/MNG images" },
        {   "da0b", "DMB MAF, extending DA0A, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "da1a", "DMB MAF audio with ER-BSAC audio, JPG/PNG/MNG images" },
        {   "da1b", "DMB MAF, extending da1a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "da2a", "DMB MAF aud w/ HE-AAC v2 aud, MOT slides, DLS, JPG/PNG/MNG images" },
        {   "da2b", "DMB MAF, extending da2a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "da3a", "DMB MAF aud with HE-AAC aud, JPG/PNG/MNG images" },
        {   "da3b", "DMB MAF, extending da3a w/ BIFS, 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dmb1", "DMB MAF supporting all the components defined in the specification" },
        {   "dmpf", "Digital Media Project" },
        {   "drc1", "Dirac (wavelet compression), encapsulated in ISO base media (MP4)" },
        {   "dv1a", "DMB MAF vid w/ AVC vid, ER-BSAC aud, BIFS, JPG/PNG/MNG images, TS" },
        {   "dv1b", "DMB MAF, extending dv1a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dv2a", "DMB MAF vid w/ AVC vid, HE-AAC v2 aud, BIFS, JPG/PNG/MNG images, TS" },
        {   "dv2b", "DMB MAF, extending dv2a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dv3a", "DMB MAF vid w/ AVC vid, HE-AAC aud, BIFS, JPG/PNG/MNG images, TS" },
        {   "dv3b", "DMB MAF, extending dv3a, with 3GPP timed text, DID, TVA, REL, IPMP" },
        {   "dvr1", "DVB (.DVB) over RTP" },
        {   "dvt1", "DVB (.DVB) over MPEG-2 Transport Stream" },
        {   "isc2", "ISMACryp 2.0 Encrypted File" },
        {   "iso2", "MP4 Base Media v2 [ISO 14496-12:2005]" },
        {   "isom", "MP4 Base Media v1 [IS0 14496-12:2003]" },
        {   "jpm ", "JPEG 2000 Compound Image (.JPM) [ISO 15444-6]" },
        {   "jpx ", "JPEG 2000 with extensions (.JPX) [ISO 15444-2]" },
        {   "mj2s", "Motion JPEG 2000 [ISO 15444-3] Simple Profile" },
        {   "mjp2", "Motion JPEG 2000 [ISO 15444-3] General Profile" },
        {   "mmp4", "MPEG-4/3GPP Mobile Profile (.MP4/3GP) (for NTT)" },
        {   "mp21", "MPEG-21 [ISO/IEC 21000-9]" },
        {   "mp41", "MP4 v1 [ISO 14496-1:ch13]" },
        {   "mp42", "MP4 v2 [ISO 14496-14]" },
        {   "mp71", "MP4 w/ MPEG-7 Metadata [per ISO 14496-12]" },
        {   "mqt ", "Sony / Mobile QuickTime (.MQV) US Patent 7,477,830 (Sony Corp)" },
        {   "niko", "Nikon" },
        {   "odcf", "OMA DCF DRM Format 2.0 (OMA-TS-DRM-DCF-V2_0-20060303-A)" },
        {   "opf2", "OMA PDCF DRM Format 2.1 (OMA-TS-DRM-DCF-V2_1-20070724-C)" },
        {   "opx2", "OMA PDCF DRM + XBS extensions (OMA-TS-DRM_XBS-V1_0-20070529-C)" },
        {   "pana", "Panasonic Digital Camera" },
        {   "qt  ", "Apple QuickTime (.MOV/QT)" },
        {   "sdv ", "SD Memory Card Video" },
        {   "ssc1", "Samsung stereoscopic, single stream" },
        {   "ssc2", "Samsung stereoscopic, dual stream" }
    };

    extern const TagVocabulary handlerClassTags[] =  {
        {   "dhlr", "Data Handler" },
        {   "mhlr", "Media Handler" }
    };

    extern const TagVocabulary handlerTypeTags[] =  {
        {   "alis", "Alias Data" },
        {   "crsm", "Clock Reference" },
        {   "hint", "Hint Track" },
        {   "ipsm", "IPMP" },
        {   "m7sm", "MPEG-7 Stream" },
        {   "mdir", "Metadata" },
        {   "mdta", "Metadata Tags" },
        {   "mjsm", "MPEG-J" },
        {   "ocsm", "Object Content" },
        {   "odsm", "Object Descriptor" },
        {   "sdsm", "Scene Description" },
        {   "soun", "Audio Track" },
        {   "text", "Text" },
        {   "tmcd", "Time Code" },
        {   "url ", "URL" },
        {   "vide", "Video Track" }
    };

    extern const TagVocabulary vendorIDTags[] =  {
        {   "FFMP", "FFmpeg" },
        {   "appl", "Apple" },
        {   "olym", "Olympus" },
        {   "GIC ", "General Imaging Co." },
        {   "fe20", "Olympus (fe20)" },
        {   "pana", "Panasonic" },
        {   "KMPI", "Konica-Minolta" },
        {   "kdak", "Kodak" },
        {   "pent", "Pentax" },
        {   "NIKO", "Nikon" },
        {   "leic", "Leica" },
        {   "pr01", "Olympus (pr01)" },
        {   "SMI ", "Sorenson Media Inc." },
        {   "mino", "Minolta" },
        {   "sany", "Sanyo" },
        {   "ZORA", "Zoran Corporation" },
        {   "niko", "Nikon" }
    };

    extern const TagVocabulary cameraByteOrderTags[] =  {
        {   "II", "Little-endian (Intel, II)" },
        {   "MM", "Big-endian (Motorola, MM)" }
    };

    extern const TagDetails graphicsModetags[] = {
        {   0x0, "srcCopy" },
        {   0x1, "srcOr" },
        {   0x2, "srcXor" },
        {   0x3, "srcBic" },
        {   0x4, "notSrcCopy" },
        {   0x5, "notSrcOr" },
        {   0x6, "notSrcXor" },
        {   0x7, "notSrcBic" },
        {   0x8, "patCopy" },
        {   0x9, "patOr" },
        {   0xa, "patXor" },
        {   0xb, "patBic" },
        {   0xc, "notPatCopy" },
        {   0xd, "notPatOr" },
        {   0xe, "notPatXor" },
        {   0xf, "notPatBic" },
        {   0x20, "blend" },
        {   0x21, "addPin" },
        {   0x22, "addOver" },
        {   0x23, "subPin" },
        {   0x24, "transparent" },
        {   0x25, "addMax" },
        {   0x26, "subOver" },
        {   0x27, "addMin" },
        {   0x31, "grayishTextOr" },
        {   0x32, "hilite" },
        {   0x40, "ditherCopy" },
        {   0x100, "Alpha" },
        {   0x101, "White Alpha" },
        {   0x102, "Pre-multiplied Black Alpha" },
        {   0x110, "Component Alpha" }
    };


    extern const TagVocabulary userDatatags[] = {
        {   "AllF", "PlayAllFrames" },
        {   "CNCV", "CompressorVersion" },
        {   "CNFV", "FirmwareVersion" },
        {   "CNMN", "Model" },
        {   "CNTH", "CanonCNTH" },
        {   "DcMD", "DcMD" },
        {   "FFMV", "FujiFilmFFMV" },
        {   "INFO", "SamsungINFO" },
        {   "LOOP", "LoopStyle" },
        {   "MMA0", "MinoltaMMA0" },
        {   "MMA1", "MinoltaMMA1" },
        {   "MVTG", "FujiFilmMVTG" },
        {   "NCDT", "NikonNCDT" },
        {   "PANA", "PanasonicPANA" },
        {   "PENT", "PentaxPENT" },
        {   "PXMN", "MakerNotePentax5b" },
        {   "PXTH", "PentaxPreview" },
        {   "QVMI", "CasioQVMI" },
        {   "SDLN", "PlayMode" },
        {   "SelO", "PlaySelection" },
        {   "TAGS", "KodakTags/KonicaMinoltaTags/MinoltaTags/NikonTags/OlympusTags/PentaxTags/SamsungTags/SanyoMOV/SanyoMP4" },
        {   "WLOC", "WindowLocation" },
        {   "XMP_", "XMP" },
        {   "Xtra", "Xtra" },
        {   "hinf", "HintTrackInfo" },
        {   "hinv", "HintVersion" },
        {   "hnti", "Hint" },
        {   "meta", "Meta" },
        {   "name", "Name" },
        {   "ptv ", "PrintToVideo" },
        {   "scrn", "OlympusPreview" },
        {   "thmb", "MakerNotePentax5a/OlympusThumbnail" },
    };

    extern const TagVocabulary userDataReferencetags[] = {
        {   "CNCV", "Xmp.video.CompressorVersion" },
        {   "CNFV", "Xmp.video.FirmwareVersion" },
        {   "CNMN", "Xmp.video.Model" },
        {   "NCHD", "Xmp.video.MakerNoteType" },
        {   "WLOC", "Xmp.video.WindowLocation" },
        {   "SDLN", "Xmp.video.PlayMode" },
        {   "SelO", "Xmp.video.PlaySelection" },
        {   "name", "Xmp.video.Name" },
        {   "vndr", "Xmp.video.Vendor" },
        {   " ART", "Xmp.video.Artist" },
        {   " alb", "Xmp.video.Album" },
        {   " arg", "Xmp.video.Arranger" },
        {   " ark", "Xmp.video.ArrangerKeywords" },
        {   " cmt", "Xmp.video.Comment" },
        {   " cok", "Xmp.video.ComposerKeywords" },
        {   " com", "Xmp.video.Composer" },
        {   " cpy", "Xmp.video.Copyright" },
        {   " day", "Xmp.video.CreateDate" },
        {   " dir", "Xmp.video.Director" },
        {   " ed1", "Xmp.video.Edit1" },
        {   " ed2", "Xmp.video.Edit2" },
        {   " ed3", "Xmp.video.Edit3" },
        {   " ed4", "Xmp.video.Edit4" },
        {   " ed5", "Xmp.video.Edit5" },
        {   " ed6", "Xmp.video.Edit6" },
        {   " ed7", "Xmp.video.Edit7" },
        {   " ed8", "Xmp.video.Edit8" },
        {   " ed9", "Xmp.video.Edit9" },
        {   " enc", "Xmp.video.Encoder" },
        {   " fmt", "Xmp.video.Format" },
        {   " gen", "Xmp.video.Genre" },
        {   " grp", "Xmp.video.Grouping" },
        {   " inf", "Xmp.video.Information" },
        {   " isr", "Xmp.video.ISRCCode" },
        {   " lab", "Xmp.video.RecordLabelName" },
        {   " lal", "Xmp.video.RecordLabelURL" },
        {   " lyr", "Xmp.video.Lyrics" },
        {   " mak", "Xmp.video.Make" },
        {   " mal", "Xmp.video.MakerURL" },
        {   " mod", "Xmp.video.Model" },
        {   " nam", "Xmp.video.Title" },
        {   " pdk", "Xmp.video.ProducerKeywords" },
        {   " phg", "Xmp.video.RecordingCopyright" },
        {   " prd", "Xmp.video.Producer" },
        {   " prf", "Xmp.video.Performers" },
        {   " prk", "Xmp.video.PerformerKeywords" },
        {   " prl", "Xmp.video.PerformerURL" },
        {   " req", "Xmp.video.Requirements" },
        {   " snk", "Xmp.video.SubtitleKeywords" },
        {   " snm", "Xmp.video.Subtitle" },
        {   " src", "Xmp.video.SourceCredits" },
        {   " swf", "Xmp.video.SongWriter" },
        {   " swk", "Xmp.video.SongWriterKeywords" },
        {   " swr", "Xmp.video.SoftwareVersion" },
        {   " too", "Xmp.video.Encoder" },
        {   " trk", "Xmp.video.Track" },
        {   " wrt", "Xmp.video.Composer" },
        {   " xyz", "Xmp.video.GPSCoordinates" },
        {   "CMbo", "Xmp.video.CameraByteOrder" },
        {   "Cmbo", "Xmp.video.CameraByteOrder" },
    };

    extern const TagDetails NikonNCTGTags[] = {
        {       0x0001, "Xmp.video.Make" },
        {       0x0002, "Xmp.video.Model" },
        {       0x0003, "Xmp.video.Software" },
        {       0x0011, "Xmp.video.CreationDate" },
        {       0x0012, "Xmp.video.DateTimeOriginal" },
        {       0x0013, "Xmp.video.FrameCount" },
        {       0x0016, "Xmp.video.FrameRate" },
        {       0x0022, "Xmp.video.FrameWidth" },
        {       0x0023, "Xmp.video.FrameHeight" },
        {       0x0032, "Xmp.audio.channelType" },
        {       0x0033, "Xmp.audio.BitsPerSample" },
        {       0x0034, "Xmp.audio.sampleRate" },
        {    0x1108822, "Xmp.video.ExposureProgram" },
        {    0x1109204, "Xmp.video.ExposureCompensation" },
        {    0x1109207, "Xmp.video.MeteringMode" },
        {    0x110a434, "Xmp.video.LensModel" },
        {    0x1200000, "Xmp.video.GPSVersionID" },
        {    0x1200001, "Xmp.video.GPSLatitudeRef" },
        {    0x1200002, "Xmp.video.GPSLatitude" },
        {    0x1200003, "Xmp.video.GPSLongitudeRef" },
        {    0x1200004, "Xmp.video.GPSLongitude" },
        {    0x1200005, "Xmp.video.GPSAltitudeRef" },
        {    0x1200006, "Xmp.video.GPSAltitude" },
        {    0x1200007, "Xmp.video.GPSTimeStamp" },
        {    0x1200008, "Xmp.video.GPSSatellites" },
        {    0x1200010, "Xmp.video.GPSImgDirectionRef" },
        {    0x1200011, "Xmp.video.GPSImgDirection" },
        {    0x1200012, "Xmp.video.GPSMapDatum" },
        {    0x120001d, "Xmp.video.GPSDateStamp" },
        {    0x2000001, "Xmp.video.MakerNoteVersion" },
        {    0x2000005, "Xmp.video.WhiteBalance" },
        {    0x200000b, "Xmp.video.WhiteBalanceFineTune" },
        {    0x200001e, "Xmp.video.ColorSpace" },
        {    0x2000023, "Xmp.video.PictureControlData" },
        {    0x2000024, "Xmp.video.WorldTime" },
        {    0x200002c, "Xmp.video.UnknownInfo" },
        {    0x2000032, "Xmp.video.UnknownInfo2" },
        {    0x2000039, "Xmp.video.LocationInfo" },
        {    0x2000083, "Xmp.video.LensType" },
        {    0x2000084, "Xmp.video.LensModel" },
        {    0x20000ab, "Xmp.video.VariProgram" },
    };

    extern const TagDetails NikonColorSpace[] = {
        {       1, "sRGB" },
        {       2, "Adobe RGB" },
    };

    extern const TagVocabulary NikonGPS_Latitude_Longitude_ImgDirection_Reference[] = {
        {   "N", "North" },
        {   "S", "South" },
        {   "E", "East" },
        {   "W", "West" },
        {   "M", "Magnetic North" },
        {   "T", "True North" },
    };

    extern const TagDetails NikonGPSAltitudeRef[] = {
        {   0, "Above Sea Level" },
        {   1, "Below Sea Level" },
    };

    extern const TagDetails NikonExposureProgram[] = {
        {   0, "Not Defined" },
        {   1, "Manual" },
        {   2, "Program AE" },
        {   3, "Aperture-priority AE" },
        {   4, "Shutter speed priority AE" },
        {   5, "Creative (Slow speed)" },
        {   6, "Action (High speed)" },
        {   7, "Portrait" },
        {   8, "Landscape" },
    };

    extern const TagDetails NikonMeteringMode[] = {
        {   0, "Unknown" },
        {   1, "Average" },
        {   2, "Center-weighted average" },
        {   3, "Spot" },
        {   4, "Multi-spot" },
        {   5, "Multi-segment" },
        {   6, "Partial" },
        { 255, "Other" },
    };

    extern const TagDetails PictureControlAdjust[] = {
        {   0, "Default Settings" },
        {   1, "Quick Adjust" },
        {   2, "Full Control" },
    };

    //! Contrast and Sharpness
    extern const TagDetails NormalSoftHard[] = {
        {   0, "Normal" },
        {   1, "Soft"   },
        {   2, "Hard"   }
    };

    //! Saturation
    extern const TagDetails Saturation[] = {
        {   0, "Normal" },
        {   1, "Low"    },
        {   2, "High"   }
    };

    //! YesNo, used for DaylightSavings
    extern const TagDetails YesNo[] = {
        {   0, "No"    },
        {   1, "Yes"   }
    };

    //! DateDisplayFormat
    extern const TagDetails DateDisplayFormat[] = {
        {   0, "Y/M/D" },
        {   1, "M/D/Y" },
        {   2, "D/M/Y" }
    };

    extern const TagDetails FilterEffect[] = {
        {   0x80, "Off" },
        {   0x81, "Yellow"    },
        {   0x82, "Orange"   },
        {   0x83, "Red"   },
        {   0x84, "Green"   },
        {   0xff, "n/a"   },
    };

    extern const TagDetails ToningEffect[] = {
        {   0x80, "B&W" },
        {   0x81, "Sepia" },
        {   0x82, "Cyanotype" },
        {   0x83, "Red" },
        {   0x84, "Yellow" },
        {   0x85, "Green" },
        {   0x86, "Blue-green" },
        {   0x87, "Blue" },
        {   0x88, "Purple-blue" },
        {   0x89, "Red-purple" },
        {   0xff, "n/a" },
    };

    enum movieHeaderTags {
        MovieHeaderVersion, CreateDate, ModifyDate, TimeScale, Duration, PreferredRate, PreferredVolume,
        PreviewTime = 18, PreviewDuration,PosterTime, SelectionTime, SelectionDuration, CurrentTime, NextTrackID
    };

    enum trackHeaderTags {
        TrackHeaderVersion, TrackCreateDate, TrackModifyDate, TrackID, TrackDuration = 5, TrackLayer = 8,
        TrackVolume, ImageWidth = 19, ImageHeight
    };

    enum mediaHeaderTags {
        MediaHeaderVersion, MediaCreateDate, MediaModifyDate, MediaTimeScale, MediaDuration, MediaLanguageCode
    };

    enum handlerTags {
        HandlerClass = 1, HandlerType, HandlerVendorID
    };

    enum videoHeaderTags {
        GraphicsMode = 2, OpColor
    };

    enum stream {
        Video, Audio, Hint, Null, GenMediaHeader
    };

    enum imageDescTags {
        codec, VendorID = 4, SourceImageWidth_Height = 7,  XResolution,
        YResolution, CompressorName = 10, BitDepth
    };

    enum audioDescTags {
        AudioFormat, AudioVendorID = 4, AudioChannels, AudioSampleRate = 7, MOV_AudioFormat = 13
    };

    bool equalsQTimeTag(Exiv2::DataBuf& buf ,const char* str2) {
        for(int i=0;i<4;i++)
            if(tolower(buf.pData_[i])!=tolower(str2[i]))
                return false;
        return true;
    }

    //! Function used to ignore Tags and values stored in them, since they are not necessary as metadata information
    //! Returns true, if Tag is found in the ignoreList[]
    bool ignoreList (Exiv2::DataBuf& buf) {
        const char ignoreList[13][5] = {
            "mdat", "edts", "junk", "iods", "alis", "stsc", "stsz", "stco", "ctts", "stss",
            "skip", "wide", "cmvd",
        };

        for(int i = 0 ; i < 13 ; i++)
            if(equalsQTimeTag(buf, ignoreList[i]))
                return true;

        return false;
    }

    //! Function used to ignore Tags, basically Tags which contain other tags inside them, since they are not necessary as metadata information
    //! Returns true, if Tag is found in the ignoreList[]
    bool dataIgnoreList (Exiv2::DataBuf& buf) {
        const char ignoreList[7][5] = {
            "moov", "mdia", "minf", "dinf", "alis", "stbl", "cmov",
        };

        for(int i = 0 ; i < 7 ; i++)
            if(equalsQTimeTag(buf, ignoreList[i]))
                return true;

        return false;
    }

    int64_t returnBufValue(Exiv2::DataBuf& buf, int n = 4) {
        int64_t temp = 0;

        for(int i = n-1; i >= 0; i--)
            temp = temp + buf.pData_[i]*(pow(256,n-i-1));
        return temp;
    }

    uint64_t returnUnsignedBufValue(Exiv2::DataBuf& buf, int n = 4) {
        uint64_t temp = 0;

        for(int i = n-1; i >= 0; i--)
            temp = temp + buf.pData_[i]*(pow(256,n-i-1));
        return temp;
    }

    bool isQuickTimeType (char a, char b, char c, char d) {
        char qTimeTags[][5] = {
            "PICT", "free", "ftyp", "junk", "mdat",
            "moov", "pict", "pnot", "skip",  "uuid", "wide"
        };

        for(int i = 0; i <= 10; i++)
            if(a == qTimeTags[i][0] && b == qTimeTags[i][1] && c == qTimeTags[i][2] && d == qTimeTags[i][3])
                return true;
        return false;
    }

    }
}

using namespace Exiv2::Internal;

namespace Exiv2 {

QuickTimeVideo::QuickTimeVideo(BasicIo::AutoPtr io)
        : Image(ImageType::qtime, mdNone, io) {
} // QuickTimeVideo::QuickTimeVideo

std::string QuickTimeVideo::mimeType() const {
    return "video/quicktime";
}

void QuickTimeVideo::writeMetadata() {
}


void QuickTimeVideo::readMetadata() {

    if (io_->open() != 0) throw Error(9, io_->path(), strError());

    // Ensure that this is the correct image type
    if (!isQTimeType(*io_, false)) {
        if (io_->error() || io_->eof()) throw Error(14);
        throw Error(3, "QuickTime");
    }

    IoCloser closer(*io_);
    clearMetadata();
    continueTraversing_ = true;

    xmpData_["Xmp.video.FileSize"] = (double)io_->size()/(double)1048576;
    xmpData_["Xmp.video.FileName"] = io_->path();
    xmpData_["Xmp.video.MimeType"] = mimeType();

        while (continueTraversing_) {
            std::cerr<<"\n";
            decodeBlock();
        }
    std::cerr<<"\n";
}

void QuickTimeVideo::decodeBlock() {
    const long bufMinSize = 4;
    DataBuf buf(bufMinSize);
    unsigned long size = 0;
    buf.pData_[4] = '\0' ;

    std::memset(buf.pData_, 0x0, buf.size_);

    io_->read(buf.pData_, 4);
    if(io_->eof()) {
        continueTraversing_ = false;
        return;
    }

    size = Exiv2::getULong(buf.pData_, bigEndian);

    io_->read(buf.pData_, 4);
    std::cerr <<"("<<std::setw(9)<<std::right<<size<<"): ";
    if(size < 8)
        return;
    tagDecoder(buf,size-8);
}

void QuickTimeVideo::tagDecoder(Exiv2::DataBuf &buf, unsigned long size) {


    std::cerr <<std::setw(10)<<std::left<<buf.pData_<<":";

    if (ignoreList(buf))
        discard(size);

    else if (dataIgnoreList(buf))
        decodeBlock();

    else if (equalsQTimeTag(buf, "ftyp"))
        fileTypeDecoder(size);

    else if (equalsQTimeTag(buf, "trak"))
        setMediaStream();

    else if (equalsQTimeTag(buf, "mvhd"))
        movieHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "tkhd"))
        trackHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "mdhd"))
        mediaHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "hdlr"))
        handlerDecoder(size);

    else if (equalsQTimeTag(buf, "vmhd"))
        videoHeaderDecoder(size);

    else if (equalsQTimeTag(buf, "udta"))
        userDataDecoder(size);

    else if (equalsQTimeTag(buf, "url ")) {
        io_->read(buf.pData_, size);
        if (currentStream_ == Video)
            xmpData_["Xmp.video.url"] = Exiv2::toString(buf.pData_);
        else if (currentStream_ == Audio)
            xmpData_["Xmp.audio.url"] = Exiv2::toString(buf.pData_);
    }

    else if (equalsQTimeTag(buf, "urn ")) {
        io_->read(buf.pData_, size);
        if (currentStream_ == Video)
            xmpData_["Xmp.video.urn"] = Exiv2::toString(buf.pData_);
        else if (currentStream_ == Audio)
            xmpData_["Xmp.audio.urn"] = Exiv2::toString(buf.pData_);
    }

    else if (equalsQTimeTag(buf, "dcom")) {
        io_->read(buf.pData_, size);
        xmpData_["Xmp.video.Compressor"] = Exiv2::toString(buf.pData_);
    }

    else if (equalsQTimeTag(buf, "smhd")) {
        io_->read(buf.pData_, 4);
        io_->read(buf.pData_, 4);
        xmpData_["Xmp.audio.balance"] = returnBufValue(buf, 2);
    }

    else if (equalsQTimeTag(buf, "dref"))
        multipleEntriesDecoder();

    else if (equalsQTimeTag(buf, "stsd"))
        sampleDesc(size);

    else if (equalsQTimeTag(buf, "stts"))
        timeToSampleDecoder();

    else {
        std::cerr<<"Unprocessed";
        discard(size);
    }
}

void QuickTimeVideo::discard(unsigned long size) {
    uint64_t cur_pos = io_->tell();
    io_->seek(cur_pos + size, BasicIo::beg);
}

void QuickTimeVideo::userDataDecoder(unsigned long size_external) {

    uint64_t cur_pos = io_->tell();
    const TagVocabulary* td;
    const TagVocabulary* tv, *tv_internal;

    const long bufMinSize = 100;
    DataBuf buf(bufMinSize);
    unsigned long size = 0, size_internal = size_external;
    std::memset(buf.pData_, 0x0, buf.size_);

    while((size_internal/4 != 0) && (size_internal > 0)) {

        buf.pData_[4] = '\0' ;
        io_->read(buf.pData_, 4);
        size = Exiv2::getULong(buf.pData_, bigEndian);
        if(size > size_internal)
            break;
        size_internal -= size;
        io_->read(buf.pData_, 4);
        std::cerr <<"("<<std::setw(9)<<std::right<<size<<"): ";

        if(buf.pData_[0] == 169)
            buf.pData_[0] = ' ';
        td = find(userDatatags, Exiv2::toString( buf.pData_));
        std::cerr <<"|"<<buf.pData_<<"|:";
        tv = find(userDataReferencetags, Exiv2::toString( buf.pData_));

        if(size == 0 || (size - 12) <= 0)
            break;

        else if(equalsQTimeTag(buf, "DcMD")  || equalsQTimeTag(buf, "NCDT"))
            userDataDecoder(size - 8);

        else if(equalsQTimeTag(buf, "NCTG"))
            NikonTagsDecoder(size - 8);

        else if(equalsQTimeTag(buf, "CNCV") || equalsQTimeTag(buf, "CNFV") || equalsQTimeTag(buf, "CNMN") || equalsQTimeTag(buf, "NCHD")) {
            io_->read(buf.pData_, size - 8);
            xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.pData_);
        }


        else if(equalsQTimeTag(buf, "CMbo") || equalsQTimeTag(buf, "Cmbo")) {
            io_->read(buf.pData_, 2);
            buf.pData_[2] = '\0' ;
            tv_internal = find(cameraByteOrderTags, Exiv2::toString( buf.pData_));
            if (tv_internal)
                xmpData_[exvGettext(tv->label_)] = exvGettext(tv_internal->label_);
            else
                xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.pData_);
        }

        else if(tv) {
            io_->read(buf.pData_, 4);
            io_->read(buf.pData_, size-12);
            xmpData_[exvGettext(tv->label_)] = Exiv2::toString(buf.pData_);
        }

        else if(td)
            tagDecoder(buf,size-8);
    }
    io_->seek(cur_pos + size_external, BasicIo::beg);
}

void QuickTimeVideo::NikonTagsDecoder(unsigned long size_external) {
    uint64_t cur_pos = io_->tell();
    DataBuf buf(100), buf2(4);
    unsigned long TagID = 0;
    unsigned short dataLength = 0, dataType = 2;
    const TagDetails* td, *td2;

    for(int i = 0 ; i < 100 ; i++) {
        io_->read(buf.pData_, 4);
        TagID = Exiv2::getULong(buf.pData_, bigEndian);
        td = find(NikonNCTGTags, TagID);

        io_->read(buf.pData_, 2);
        dataType = Exiv2::getUShort(buf.pData_, bigEndian);

        std::memset(buf.pData_, 0x0, buf.size_);
        io_->read(buf.pData_, 2);

        if(TagID == 0x2000023) {
            uint64_t local_pos = io_->tell();
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
            std::memset(buf.pData_, 0x0, buf.size_);

            io_->read(buf.pData_, 4);   xmpData_["Xmp.video.PictureControlVersion"]  = Exiv2::toString(buf.pData_);
            io_->read(buf.pData_, 20);  xmpData_["Xmp.video.PictureControlName"]     = Exiv2::toString(buf.pData_);
            io_->read(buf.pData_, 20);  xmpData_["Xmp.video.PictureControlBase"]     = Exiv2::toString(buf.pData_);
            io_->read(buf.pData_, 4);   std::memset(buf.pData_, 0x0, buf.size_);

            io_->read(buf.pData_, 1);
            td2 = find(PictureControlAdjust, (int)buf.pData_[0] & 7 );
            if(td2)
                xmpData_["Xmp.video.PictureControlAdjust"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.PictureControlAdjust"] = (int)buf.pData_[0] & 7 ;

            io_->read(buf.pData_, 1);
            td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
            if(td2)
                xmpData_["Xmp.video.PictureControlQuickAdjust"] = exvGettext(td2->label_);

            io_->read(buf.pData_, 1);
            td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
            if(td2)
                xmpData_["Xmp.video.Sharpness"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.Sharpness"] = (int)buf.pData_[0] & 7;

            io_->read(buf.pData_, 1);
            td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
            if(td2)
                xmpData_["Xmp.video.Contrast"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.Contrast"] = (int)buf.pData_[0] & 7;

            io_->read(buf.pData_, 1);
            td2 = find(NormalSoftHard, (int)buf.pData_[0] & 7 );
            if(td2)
                xmpData_["Xmp.video.Brightness"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.Brightness"] = (int)buf.pData_[0] & 7;

            io_->read(buf.pData_, 1);
            td2 = find(Saturation, (int)buf.pData_[0] & 7 );
            if(td2)
                xmpData_["Xmp.video.Saturation"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.Saturation"] = (int)buf.pData_[0] & 7;

            io_->read(buf.pData_, 1);
            xmpData_["Xmp.video.HueAdjustment"] = (int)buf.pData_[0] & 7;

            io_->read(buf.pData_, 1);
            td2 = find(FilterEffect, (int)buf.pData_[0]);
            if(td2)
                xmpData_["Xmp.video.FilterEffect"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.FilterEffect"] = (int)buf.pData_[0];

            io_->read(buf.pData_, 1);
            td2 = find(ToningEffect, (int)buf.pData_[0]);
            if(td2)
                xmpData_["Xmp.video.ToningEffect"] = exvGettext(td2->label_);
            else
                xmpData_["Xmp.video.ToningEffect"] = (int)buf.pData_[0];

            io_->read(buf.pData_, 1);   xmpData_["Xmp.video.ToningSaturation"] = (int)buf.pData_[0];

            io_->seek(local_pos + dataLength, BasicIo::beg);
        }

        else if(TagID == 0x2000024) {
            uint64_t local_pos = io_->tell();
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
            std::memset(buf.pData_, 0x0, buf.size_);

            io_->read(buf.pData_, 2);   xmpData_["Xmp.video.TimeZone"] = Exiv2::getShort(buf.pData_, bigEndian);
            io_->read(buf.pData_, 1);
            td2 = find(YesNo, (int)buf.pData_[0]);
            if(td2)
                xmpData_["Xmp.video.DayLightSavings"] = exvGettext(td2->label_);

            io_->read(buf.pData_, 1);
            td2 = find(DateDisplayFormat, (int)buf.pData_[0]);
            if(td2)
                xmpData_["Xmp.video.DateDisplayFormat"] = exvGettext(td2->label_);

            io_->seek(local_pos + dataLength, BasicIo::beg);
        }

        else if(dataType == 2 || dataType == 7) {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian);
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, dataLength);
            if(td)
                xmpData_[exvGettext(td->label_)] = Exiv2::toString(buf.pData_);
        }
        else if(dataType == 4)  {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 4;
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 4);
            if(td)
                xmpData_[exvGettext(td->label_)] = Exiv2::toString(Exiv2::getULong( buf.pData_, bigEndian));
            io_->read(buf.pData_, dataLength - 4);
        }
        else if(dataType == 3)  {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 2;
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            if(td)
                xmpData_[exvGettext(td->label_)] = Exiv2::toString(Exiv2::getUShort( buf.pData_, bigEndian));
            io_->read(buf.pData_, dataLength - 2);
        }
        else if(dataType == 5) {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 8;
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 4);
            io_->read(buf2.pData_, 4);
            if(td)
                xmpData_[exvGettext(td->label_)] = Exiv2::toString((double)Exiv2::getULong( buf.pData_, bigEndian) / (double)Exiv2::getULong( buf2.pData_, bigEndian));
            io_->read(buf.pData_, dataLength - 8);
        }
        else if(dataType == 8) {
            dataLength = Exiv2::getUShort(buf.pData_, bigEndian) * 2;
            std::memset(buf.pData_, 0x0, buf.size_);
            io_->read(buf.pData_, 2);
            io_->read(buf2.pData_, 2);
            if(td)
                xmpData_[exvGettext(td->label_)] = Exiv2::toString(Exiv2::getUShort( buf.pData_, bigEndian) ) + " " + Exiv2::toString(Exiv2::getUShort( buf2.pData_, bigEndian));
            io_->read(buf.pData_, dataLength - 4);
        }

    }
    io_->seek(cur_pos + size_external, BasicIo::beg);
}

void QuickTimeVideo::setMediaStream() {
    uint64_t current_position = io_->tell();
    DataBuf buf(4);

    while(!io_->eof()) {
        io_->read(buf.pData_, 4);
        if (equalsQTimeTag(buf, "hdlr")) {
            io_->read(buf.pData_, 4);
            io_->read(buf.pData_, 4);
            io_->read(buf.pData_, 4);

            if (equalsQTimeTag(buf, "vide"))
                currentStream_ = Video;
            else if(equalsQTimeTag(buf, "soun"))
                currentStream_ = Audio;
            else if (equalsQTimeTag(buf, "hint"))
                currentStream_ = Hint;
            else
                currentStream_ = GenMediaHeader;
            break;
        }
    }
    io_->seek(current_position, BasicIo::beg);
}



void QuickTimeVideo::timeToSampleDecoder() {
    DataBuf buf(4);
    io_->read(buf.pData_, 4);
    io_->read(buf.pData_, 4);
    uint64_t noOfEntries, totalframes = 0, timeOfFrames = 0;
    noOfEntries = returnUnsignedBufValue(buf);
    std::cerr<<": "<<noOfEntries;
    int temp;

    for(unsigned long i = 1; i <= noOfEntries; i++) {
        io_->read(buf.pData_, 4);
        temp = returnBufValue(buf);
        totalframes += temp;
        io_->read(buf.pData_, 4);
        timeOfFrames += temp * returnBufValue(buf);
    }
    if (currentStream_ == Video)
    xmpData_["Xmp.video.frameRate"] = (double)totalframes * (double)timeScale_ / (double)timeOfFrames;
}

void QuickTimeVideo::sampleDesc(unsigned long size) {
    DataBuf buf(100);
    uint64_t cur_pos = io_->tell();
    io_->read(buf.pData_, 4);
    io_->read(buf.pData_, 4);
    uint64_t noOfEntries;
    noOfEntries = returnUnsignedBufValue(buf);

    for(unsigned long i = 1; i <= noOfEntries; i++) {
        if (currentStream_ == Video)
            imageDescDecoder();
        else if (currentStream_ == Audio)
            audioDescDecoder();

    }
    io_->seek(cur_pos + size, BasicIo::beg);

}

void QuickTimeVideo::audioDescDecoder() {
    DataBuf buf(40);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);
    uint64_t size = 82;

    const TagVocabulary* td;

    for (int i = 0; size/4 != 0 ; size -= 4, i++) {
        io_->read(buf.pData_, 4);
        switch(i) {
        case AudioFormat:
            td = find(qTimeFileType, Exiv2::toString( buf.pData_));
            if(td)
                xmpData_["Xmp.audio.compressor"] = exvGettext(td->label_);
            else
                xmpData_["Xmp.audio.compressor"] = Exiv2::toString( buf.pData_);
            break;
        case AudioVendorID:
            td = find(vendorIDTags, Exiv2::toString( buf.pData_));
            if(td)
                xmpData_["Xmp.audio.vendorID"] = exvGettext(td->label_);
            break;
        case AudioChannels:
            xmpData_["Xmp.audio.channelType"] = returnBufValue(buf, 2);
            xmpData_["Xmp.audio.BitsPerSample"] = (buf.pData_[2] * 256 + buf.pData_[3]);
            break;
        case AudioSampleRate:
            xmpData_["Xmp.audio.sampleRate"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
            break;
        default:
            break;
        }
    }
    io_->read(buf.pData_, size % 4);
}

void QuickTimeVideo::imageDescDecoder() {
    DataBuf buf(40);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    io_->read(buf.pData_, 4);
    uint64_t size = 82;

    const TagVocabulary* td;

    for (int i = 0; size/4 != 0 ; size -= 4, i++) {
        io_->read(buf.pData_, 4);

        switch(i) {
        case codec:
            td = find(qTimeFileType, Exiv2::toString( buf.pData_));
            if(td)
                xmpData_["Xmp.video.codec"] = exvGettext(td->label_);
            else
                xmpData_["Xmp.video.codec"] = Exiv2::toString( buf.pData_);
            break;
        case VendorID:
            td = find(vendorIDTags, Exiv2::toString( buf.pData_));
            if(td)
                xmpData_["Xmp.video.vendorID"] = exvGettext(td->label_);
            break;
        case SourceImageWidth_Height:
            xmpData_["Xmp.video.sourceImageWidth"] = returnBufValue(buf, 2);
            xmpData_["Xmp.video.sourceImageHeight"] = (buf.pData_[2] * 256 + buf.pData_[3]);
            break;
        case XResolution:
            xmpData_["Xmp.video.XResolution"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
            break;
        case YResolution:
            xmpData_["Xmp.video.YResolution"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
            io_->read(buf.pData_, 3); size -= 3;
            break;
        case CompressorName:
            io_->read(buf.pData_, 32); size -= 32;
            xmpData_["Xmp.video.compressor"] = Exiv2::toString( buf.pData_);
            break;
        default:
            break;
        }
    }
    io_->read(buf.pData_, size % 4);
    xmpData_["Xmp.video.bitDepth"] = returnBufValue(buf, 1);
}

void QuickTimeVideo::multipleEntriesDecoder() {
    DataBuf buf(4);
    io_->read(buf.pData_, 4);
    io_->read(buf.pData_, 4);
    uint64_t noOfEntries;

    noOfEntries = returnUnsignedBufValue(buf);

    for(unsigned long i = 1; i <= noOfEntries; i++)
        decodeBlock();
}

void QuickTimeVideo::videoHeaderDecoder(unsigned long size) {
    DataBuf buf(2);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[2] = '\0';
    currentStream_ = Video;

    const TagDetails* td;

    for (int i = 0; size/2 != 0 ; size -= 2, i++) {
        io_->read(buf.pData_, 2);

        switch(i) {
        case GraphicsMode:
            td = find(graphicsModetags, returnBufValue(buf,2));
            if(td)
                xmpData_["Xmp.video.graphicsMode"] = exvGettext(td->label_);
            break;
        case OpColor:
            xmpData_["Xmp.video.opColor"] = returnBufValue(buf,2);
            break;
        default:
            break;
        }
    }
    io_->read(buf.pData_, size % 2);
}

void QuickTimeVideo::handlerDecoder(unsigned long size) {
    uint64_t cur_pos = io_->tell();
    DataBuf buf(100);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';

    const TagVocabulary* tv;

    for (int i = 0; i < 5 ; i++) {

        io_->read(buf.pData_, 4);

        switch(i) {
        case HandlerClass:
            tv = find(handlerClassTags, Exiv2::toString( buf.pData_));
            if(tv) {
                if (currentStream_ == Video)
                    xmpData_["Xmp.video.handlerClass"] = exvGettext(tv->label_);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.handlerClass"] = exvGettext(tv->label_);
            }
            break;
        case HandlerType:
            tv = find(handlerTypeTags, Exiv2::toString( buf.pData_));
            if(tv) {
                if (currentStream_ == Video)
                    xmpData_["Xmp.video.handlerType"] = exvGettext(tv->label_);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.handlerType"] = exvGettext(tv->label_);
            }
            break;
        case HandlerVendorID:
            tv = find(vendorIDTags, Exiv2::toString( buf.pData_));
            if(tv) {
                if (currentStream_ == Video)
                    xmpData_["Xmp.video.handlerVendorID"] = exvGettext(tv->label_);
                else if (currentStream_ == Audio)
                    xmpData_["Xmp.audio.handlerVendorID"] = exvGettext(tv->label_);
            }
            break;
        }
    }
    io_->seek(cur_pos + size, BasicIo::beg);
}

void QuickTimeVideo::fileTypeDecoder(unsigned long size) {
    DataBuf buf(4);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    Exiv2::Value::AutoPtr v = Exiv2::Value::create(Exiv2::xmpSeq);
    const TagVocabulary* td;

    for (int i = 0; size/4 != 0; size -=4, i++) {
        io_->read(buf.pData_, 4);
        td = find(qTimeFileType, Exiv2::toString( buf.pData_));

        switch(i) {
        case 0:
            if(td)
            xmpData_["Xmp.video.majorBrand"] = exvGettext(td->label_);
            break;
        case 1:
            xmpData_["Xmp.video.minorVersion"] = returnBufValue(buf);
            break;
        default:
            if(td)
                v->read(exvGettext(td->label_));
            else
                v->read(Exiv2::toString(buf.pData_));
            break;
        }
    }
    xmpData_.add(Exiv2::XmpKey("Xmp.video.compatibleBrands"), v.get());
    io_->read(buf.pData_, size%4);
}

void QuickTimeVideo::mediaHeaderDecoder(unsigned long size) {
    DataBuf buf(4);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';
    int time_scale = 1;

    for (int i = 0; size/4 != 0 ; size -=4, i++) {
        io_->read(buf.pData_, 4);

        switch(i) {
        case MediaHeaderVersion:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.mediaHeaderVersion"] = returnBufValue(buf,1);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.mediaHeaderVersion"] = returnBufValue(buf,1);
            break;
        case MediaCreateDate:
            //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
            if(currentStream_ == Video)
                xmpData_["Xmp.video.mediaCreateDate"] = returnUnsignedBufValue(buf);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.mediaCreateDate"] = returnUnsignedBufValue(buf);
            break;
        case MediaModifyDate:
            //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
            if(currentStream_ == Video)
                xmpData_["Xmp.video.mediaModifyDate"] = returnUnsignedBufValue(buf);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.mediaModifyDate"] = returnUnsignedBufValue(buf);
            break;
        case MediaTimeScale:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.mediaTimeScale"] = returnBufValue(buf);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.mediaTimeScale"] = returnBufValue(buf);
            time_scale = returnBufValue(buf);
            break;
        case MediaDuration:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.mediaDuration"] = returnBufValue(buf)/time_scale;
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.mediaDuration"] = returnBufValue(buf)/time_scale;
            break;
        case MediaLanguageCode:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.mediaLangCode"] = returnUnsignedBufValue(buf,2);
            else if (currentStream_ == Audio)
                xmpData_["Xmp.audio.mediaLangCode"] = returnUnsignedBufValue(buf,2);
            break;
        default:
            break;

        }
    }
    io_->read(buf.pData_, size%4);
}

void QuickTimeVideo::trackHeaderDecoder(unsigned long size) {
    DataBuf buf(4);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';

    std::cerr<<"    Stream  |"<<currentStream_<<"|  ";

    for (int i = 0; size/4 != 0  ; size -=4, i++) {
        io_->read(buf.pData_, 4);

        switch(i) {
        case TrackHeaderVersion:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackHeaderVersion"] = returnBufValue(buf,1);
            else if(currentStream_ == Audio)
                xmpData_["Xmp.audio.trackHeaderVersion"] = returnBufValue(buf,1);
            break;
        case TrackCreateDate:
            //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackCreateDate"] = returnUnsignedBufValue(buf);
            else if(currentStream_ == Audio)
                xmpData_["Xmp.audio.trackCreateDate"] = returnUnsignedBufValue(buf);
            break;
        case TrackModifyDate:
            //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackModifyDate"] = returnUnsignedBufValue(buf);
            else if(currentStream_ == Audio)
                xmpData_["Xmp.audio.trackModifyDate"] = returnUnsignedBufValue(buf);
            break;
        case TrackID:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackID"] = returnBufValue(buf);
            else if(currentStream_ == Audio)
                xmpData_["Xmp.audio.trackID"] = returnBufValue(buf);
            break;
        case TrackDuration:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackDuration"] = returnBufValue(buf)/timeScale_;
            else if(currentStream_ == Audio)
                xmpData_["Xmp.audio.trackDuration"] = returnBufValue(buf)/timeScale_;
            break;
        case TrackLayer:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackLayer"] = returnBufValue(buf, 2);
            else if(currentStream_ == Audio)
                xmpData_["Xmp.audio.trackLayer"] = returnBufValue(buf, 2);
            break;
        case TrackVolume:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.trackVolume"] = (returnBufValue(buf, 1) + (buf.pData_[2] * 0.1)) * 100;
            else if(currentStream_ == Audio)
                xmpData_["Xmp.video.trackVolume"] = (returnBufValue(buf, 1) + (buf.pData_[2] * 0.1)) * 100;
            break;
        case ImageWidth:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.width"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
            break;
        case ImageHeight:
            if(currentStream_ == Video)
                xmpData_["Xmp.video.height"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01);
            break;
        default:
            break;
        }
    }
    io_->read(buf.pData_, size%4);
}

void QuickTimeVideo::movieHeaderDecoder(unsigned long size) {
    DataBuf buf(4);
    std::memset(buf.pData_, 0x0, buf.size_);
    buf.pData_[4] = '\0';

    for (int i = 0; size/4 != 0 ; size -=4, i++) {
        io_->read(buf.pData_, 4);

        switch(i) {
        case MovieHeaderVersion:
            xmpData_["Xmp.video.movieHeaderVersion"] = returnBufValue(buf,1); break;
        case CreateDate:
            //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
            xmpData_["Xmp.video.dateUTC"] = returnUnsignedBufValue(buf); break;
        case ModifyDate:
            //A 32-bit integer that specifies (in seconds since midnight, January 1, 1904) when the movie atom was created.
            xmpData_["Xmp.video.modificationDate"] = returnUnsignedBufValue(buf); break;
        case TimeScale:
            xmpData_["Xmp.video.timeScale"] = returnBufValue(buf);
            timeScale_ = returnBufValue(buf); break;
        case Duration:
            xmpData_["Xmp.video.duration"] = returnBufValue(buf) * 1000 / timeScale_; break;
        case PreferredRate:
            xmpData_["Xmp.video.preferredRate"] = returnBufValue(buf, 2) + ((buf.pData_[2] * 256 + buf.pData_[3]) * 0.01); break;
        case PreferredVolume:
            xmpData_["Xmp.video.preferredVolume"] = (returnBufValue(buf, 1) + (buf.pData_[2] * 0.1)) * 100; break;
        case PreviewTime:
            xmpData_["Xmp.video.previewTime"] = returnBufValue(buf); break;
        case PreviewDuration:
            xmpData_["Xmp.video.previewDuration"] = returnBufValue(buf); break;
        case PosterTime:
            xmpData_["Xmp.video.posterTime"] = returnBufValue(buf); break;
        case SelectionTime:
            xmpData_["Xmp.video.selectionTime"] = returnBufValue(buf); break;
        case SelectionDuration:
            xmpData_["Xmp.video.selectionDuration"] = returnBufValue(buf); break;
        case CurrentTime:
            xmpData_["Xmp.video.currentTime"] = returnBufValue(buf); break;
        case NextTrackID:
            xmpData_["Xmp.video.nextTrackID"] = returnBufValue(buf); break;
        default:
            break;
        }
    }
    io_->read(buf.pData_, size%4);
}

Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool /*create*/) {
    Image::AutoPtr image(new QuickTimeVideo(io));
    if (!image->good()) {
        image.reset();
    }
    return image;
}

bool isQTimeType(BasicIo& iIo, bool advance) {
    const int32_t len = 4;
    byte buf[len];
    iIo.read(buf, len);
    iIo.read(buf, len);

    if (iIo.error() || iIo.eof()) {
        return false;
    }

    bool matched = isQuickTimeType(buf[0], buf[1], buf[2], buf[3]);
    if (!advance || !matched) {
        iIo.seek(0, BasicIo::beg);
    }

    return matched;
}

}
