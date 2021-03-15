//
//  value.h
//  docopt
//
//  Created by Jared Grubb on 2013-10-14.
//  Copyright (c) 2013 Jared Grubb. All rights reserved.
//

#ifndef DOCOPT_VALUE_H
#define DOCOPT_VALUE_H

#include <string>
#include <vector>
#include <functional> // std::hash
#include <iosfwd>
#include <stdexcept>

namespace docopt {

	enum class Kind {
		Empty,
		Bool,
		Long,
		String,
		StringList
	};

	/// A generic type to hold the various types that can be produced by docopt.
	///
	/// This type can be one of: {bool, long, string, vector<string>}, or empty.
	struct value {
		/// An empty value
		value();

		value(std::string);
		value(std::vector<std::string>);
		
		explicit value(bool);
		explicit value(long);
		explicit value(int);

		~value();
		value(value const&);
		value(value&&) noexcept;
		value& operator=(value const&);
		value& operator=(value&&) noexcept;

		Kind kind() const;
		
		// Test if this object has any contents at all
		explicit operator bool() const;
		
		// Test the type contained by this value object
		bool isBool() const;
		bool isString() const;
		bool isLong() const;
		bool isStringList() const;

		// Throws std::invalid_argument if the type does not match
		bool asBool() const;
		long asLong() const;
		std::string const& asString() const;
		std::vector<std::string> const& asStringList() const;

		size_t hash() const noexcept;
		
		friend bool operator==(value const&, value const&);
		friend bool operator!=(value const&, value const&);

	private:
		union Variant {
			Variant();
			~Variant();
			
			bool boolValue;
			long longValue;
			std::string strValue;
			std::vector<std::string> strList;
		};
		
		static const char* kindAsString(Kind kind);

		void throwIfNotKind(Kind expected) const;

		Kind kind_ = Kind::Empty;
		Variant variant_;
	};

	/// Write out the contents to the ostream
	std::ostream& operator<<(std::ostream&, value const&);
}

namespace std {
	template <>
	struct hash<docopt::value> {
		size_t operator()(docopt::value const& val) const noexcept {
			return val.hash();
		}
	};
}

#endif /* defined(DOCOPT_VALUE_H) */
