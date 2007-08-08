// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      xmp.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   13-July-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "xmp.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "properties.hpp"

// + standard includes
#include <iostream>
#include <algorithm>
#include <cassert>
#include <string>

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
# define TXMP_STRING_TYPE std::string
# include <XMP.hpp>
# include <XMP.incl_cpp>
#endif // EXV_HAVE_XMP_TOOLKIT

// *****************************************************************************
namespace {
    //! Unary predicate that matches an Xmpdatum by key
    class FindXmpdatum {
    public:
        //! Constructor, initializes the object with key
        FindXmpdatum(const Exiv2::XmpKey& key)
            : key_(key.key()) {}
        /*!
          @brief Returns true if prefix and property of the argument
                 Xmpdatum are equal to that of the object.
        */
        bool operator()(const Exiv2::Xmpdatum& xmpdatum) const
            { return key_ == xmpdatum.key(); }

    private:
        std::string key_;

    }; // class FindXmpdatum

    //! Make an XMP key from a schema namespace and property path
    Exiv2::XmpKey::AutoPtr makeXmpKey(const std::string& schemaNs,
                                      const std::string& propPath);

    // Todo: This should be part of the API
    //! Quote a string with double-quotes, escape quotes in the string
    void quoteText(std::string& text);

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE

    //! Internal Pimpl structure with private members and data of class Xmpdatum.
    struct Xmpdatum::Impl {
        Impl(const XmpKey& key, const Value* pValue);  //!< Constructor
        Impl(const Impl& rhs);                         //!< Copy constructor
        Impl& operator=(const Impl& rhs);              //!< Assignment

        // DATA
        XmpKey::AutoPtr key_;                          //!< Key
        Value::AutoPtr  value_;                        //!< Value
    };
    //! @endcond

    Xmpdatum::Impl::Impl(const XmpKey& key, const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Xmpdatum::Impl::Impl(const Impl& rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    Xmpdatum::Impl::Impl& Xmpdatum::Impl::operator=(const Impl& rhs)
    {
        if (this == &rhs) return *this;
        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
        return *this;
    }

    Xmpdatum::Xmpdatum(const XmpKey& key, const Value* pValue)
        : p_(new Impl(key, pValue))
    {
    }

    Xmpdatum::Xmpdatum(const Xmpdatum& rhs)
        : Metadatum(rhs), p_(new Impl(*rhs.p_))
    {
    }

    Xmpdatum& Xmpdatum::operator=(const Xmpdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);
        *p_ = *rhs.p_;
        return *this;
    }

    Xmpdatum::~Xmpdatum()
    {
        delete p_;
    }

    std::string Xmpdatum::key() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->key(); 
    }

    std::string Xmpdatum::groupName() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->groupName();
    }

    std::string Xmpdatum::tagName() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->tagName();
    }

    std::string Xmpdatum::tagLabel() const
    {
        return p_->key_.get() == 0 ? "" : p_->key_->tagLabel();
    }

    uint16_t Xmpdatum::tag() const
    {
        return p_->key_.get() == 0 ? 0 : p_->key_->tag();
    }

    TypeId Xmpdatum::typeId() const
    {
        return p_->value_.get() == 0 ? invalidTypeId : p_->value_->typeId();
    }

    const char* Xmpdatum::typeName() const
    {
        return TypeInfo::typeName(typeId());
    }

    long Xmpdatum::count() const
    {
        return p_->value_.get() == 0 ? 0 : p_->value_->count();
    }

    long Xmpdatum::size() const
    {
        return p_->value_.get() == 0 ? 0 : p_->value_->size();
    }

    std::string Xmpdatum::toString() const
    {
        return p_->value_.get() == 0 ? "" : p_->value_->toString();
    }

    long Xmpdatum::toLong(long n) const
    {
        return p_->value_.get() == 0 ? -1 : p_->value_->toLong(n);
    }

    float Xmpdatum::toFloat(long n) const
    {
        return p_->value_.get() == 0 ? -1 : p_->value_->toFloat(n); 
    }

    Rational Xmpdatum::toRational(long n) const
    {
        return p_->value_.get() == 0 ? Rational(-1, 1) : p_->value_->toRational(n);
    }

    Value::AutoPtr Xmpdatum::getValue() const
    {
        return p_->value_.get() == 0 ? Value::AutoPtr(0) : p_->value_->clone(); 
    }

    const Value& Xmpdatum::value() const
    {
        if (p_->value_.get() == 0) throw Error(8);
        return *p_->value_;
    }

    long Xmpdatum::copy(byte* /*buf*/, ByteOrder /*byteOrder*/) const
    {
        throw Error(34, "Xmpdatum::copy");
        return 0;
    }

    Xmpdatum& Xmpdatum::operator=(const uint16_t& value)
    {
        UShortValue::AutoPtr v(new UShortValue);
        v->value_.push_back(value);
        p_->value_ = v;
        return *this;
    }

    Xmpdatum& Xmpdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Xmpdatum& Xmpdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Xmpdatum::setValue(const Value* pValue)
    {
        p_->value_.reset();
        if (pValue) p_->value_ = pValue->clone();
    }

    void Xmpdatum::setValue(const std::string& value)
    {
        // Todo: What's the correct default? Adjust doc
        if (p_->value_.get() == 0) {
            assert(0 != p_->key_.get());
            TypeId type = XmpProperties::propertyType(*p_->key_.get());
            p_->value_ = Value::create(type);
        }
        p_->value_->read(value);
    }

    Xmpdatum& XmpData::operator[](const std::string& key)
    {
        XmpKey xmpKey(key);
        iterator pos = findKey(xmpKey);
        if (pos == end()) {
            add(Xmpdatum(xmpKey));
            pos = findKey(xmpKey);
        }
        return *pos;
    }

#ifdef EXV_HAVE_XMP_TOOLKIT
    int XmpData::load(const byte* buf, long len)
    {
        xmpMetadata_.clear();

        SXMPMeta meta(reinterpret_cast<const char*>(buf), len);
        SXMPIterator iter(meta);
        std::string schemaNs, propPath, propValue;
        XMP_OptionBits opt;
        while (iter.Next(&schemaNs, &propPath, &propValue, &opt)) {
            if (XMP_NodeIsSchema(opt)) continue;

            XmpKey::AutoPtr key = makeXmpKey(schemaNs, propPath);
            if (key.get() == 0) continue;

            // Create an Exiv2 value and read the property value
            Value::AutoPtr val = Value::create(XmpProperties::propertyType(*key.get()));
            if (XMP_PropIsSimple(opt)) {
                if (val->typeId() != xmpText) {
                    int ret = val->read(propValue);
                    // Todo: Exiv2 ValueType<T>::read should check the string 
                    //       and not just return 0
                    if (ret != 0) val = Value::create(xmpText);
                }
                if (val->typeId() == xmpText) {
                    std::string pv = propValue;
                    quoteText(pv);
                    val->read(pv);
                }
            }
            else if (XMP_PropIsArray(opt)) {
                XMP_Index itemIdx = 1;
                std::string itemValue, arrayValue;
                XMP_OptionBits itemOpt;
                while (meta.GetArrayItem(schemaNs.c_str(), propPath.c_str(),
                                         itemIdx, &itemValue, &itemOpt)) {
                    if (val->typeId() == xmpText) quoteText(itemValue);
                    if (itemIdx > 1) arrayValue += " ";
                    arrayValue += itemValue;
                    ++itemIdx;
                }
                iter.Skip(kXMP_IterSkipSubtree);
                val->read(arrayValue);
            }
            else {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: XMP property " << key->key()
                          << " has unsupported property type; skipping property.\n";
#endif
                iter.Skip(kXMP_IterSkipSubtree);
                continue;
            }

            add(*key.get(), val.get());
        }

        return 0;
    } // XmpData::load
#else
    int XmpData::load(const byte* /*buf*/, long /*len*/)
    {
#ifndef SUPPRESS_WARNINGS
        std::cerr << "Warning: XMP toolkit support not compiled in.\n";
#endif
        return 1;
    } // XmpData::load
#endif // !EXV_HAVE_XMP_TOOLKIT

#ifdef EXV_HAVE_XMP_TOOLKIT
    DataBuf XmpData::copy() const
    {
        DataBuf buf;

        // Todo: Implement me!

        return buf;
    } // XmpData::copy
#else
    DataBuf XmpData::copy() const
    {
#ifndef SUPPRESS_WARNINGS
        std::cerr << "Warning: XMP toolkit support not compiled in.\n";
#endif
        return DataBuf();
    } // XmpData::copy
#endif // !EXV_HAVE_XMP_TOOLKIT

    int XmpData::add(const XmpKey& key, Value* value)
    {
        return add(Xmpdatum(key, value));
    }

    int XmpData::add(const Xmpdatum& xmpDatum)
    {
        xmpMetadata_.push_back(xmpDatum);
        return 0;
    }

    XmpData::const_iterator XmpData::findKey(const XmpKey& key) const
    {
        return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(),
                            FindXmpdatum(key));
    }

    XmpData::iterator XmpData::findKey(const XmpKey& key)
    {
        return std::find_if(xmpMetadata_.begin(), xmpMetadata_.end(),
                            FindXmpdatum(key));
    }

    void XmpData::clear()
    {
        xmpMetadata_.clear();
    }

    void XmpData::sortByKey()
    {
        std::sort(xmpMetadata_.begin(), xmpMetadata_.end(), cmpMetadataByKey);
    }

    XmpData::const_iterator XmpData::begin() const 
    {
        return xmpMetadata_.begin(); 
    }

    XmpData::const_iterator XmpData::end() const
    {
        return xmpMetadata_.end(); 
    }

    bool XmpData::empty() const 
    {
        return count() == 0;
    }

    long XmpData::count() const
    {
        return static_cast<long>(xmpMetadata_.size());
    }

    XmpData::iterator XmpData::begin()
    {
        return xmpMetadata_.begin();
    }

    XmpData::iterator XmpData::end()
    {
        return xmpMetadata_.end();
    }

    XmpData::iterator XmpData::erase(XmpData::iterator pos)
    {
        return xmpMetadata_.erase(pos);
    }

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const Xmpdatum& md)
    {
        return os << md.value();
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {
    Exiv2::XmpKey::AutoPtr makeXmpKey(const std::string& schemaNs,
                                      const std::string& propPath)
    {
        std::string property;
        std::string::size_type idx = propPath.find(':');
        if (idx != std::string::npos) {
            // Don't worry about out_of_range, XMP parser takes care of this
            property = propPath.substr(idx + 1);
        }
        else {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to determine property name from path "
                      << propPath << ", namespace " << schemaNs 
                      << "; skipping property.\n";
#endif
            return Exiv2::XmpKey::AutoPtr();
        }
        const char* prefix = Exiv2::XmpProperties::ns(schemaNs);
        if (prefix == 0) {
#ifndef SUPPRESS_WARNINGS
            // Todo: Print warning only for the first property in each ns
            std::cerr << "Warning: Unknown schema namespace " 
                      << schemaNs << "; skipping property "
                      << property << ".\n";
#endif
            return Exiv2::XmpKey::AutoPtr();
        }
        Exiv2::XmpKey::AutoPtr key;
        // Todo: Avoid the try/catch block
        try {
            key = Exiv2::XmpKey::AutoPtr(new Exiv2::XmpKey(prefix, property));
        }
        catch (const Exiv2::AnyError& e) {
            // This should only happen for unknown property names
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: " << e << "; skipping property.\n";
#endif
        }
        return key;
    } // makeXmpKey

    void quoteText(std::string& text) 
    {
        const char quote = '\"';
        for (std::string::size_type pos = text.find(quote);
             pos != std::string::npos;
             pos = text.find(quote, pos + 2)) {

            text.insert(pos, "\\");

        }
        text = quote + text + quote;
    } // quoteText

}