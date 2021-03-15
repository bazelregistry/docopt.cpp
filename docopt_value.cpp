#include "docopt_value.h"
#include "docopt_config.h"
#include "docopt_util.h"
#include "docopt_private.h"

DOCOPT_INLINE
docopt::value::value() {}

DOCOPT_INLINE
docopt::value::value(bool v)
: kind_(Kind::Bool)
{
	variant_.boolValue = v;
}

DOCOPT_INLINE
docopt::value::value(long v)
: kind_(Kind::Long)
{
	variant_.longValue = v;
}

DOCOPT_INLINE
docopt::value::value(int v)
: value(static_cast<long>(v))
{
}

DOCOPT_INLINE
docopt::value::value(std::string v)
: kind_(Kind::String)
{
	new (&variant_.strValue) std::string(std::move(v));
}

DOCOPT_INLINE
docopt::value::value(std::vector<std::string> v)
: kind_(Kind::StringList)
{
	new (&variant_.strList) std::vector<std::string>(std::move(v));
}

DOCOPT_INLINE
docopt::value::value(value const& other)
: kind_(other.kind_)
{
	switch (kind_) {
		case Kind::String:
			new (&variant_.strValue) std::string(other.variant_.strValue);
			break;

		case Kind::StringList:
			new (&variant_.strList) std::vector<std::string>(other.variant_.strList);
			break;

		case Kind::Bool:
			variant_.boolValue = other.variant_.boolValue;
			break;

		case Kind::Long:
			variant_.longValue = other.variant_.longValue;
			break;

		case Kind::Empty:
		default:
			break;
	}
}

DOCOPT_INLINE
docopt::value::value(value&& other) noexcept
: kind_(other.kind_)
{
	switch (kind_) {
		case Kind::String:
			new (&variant_.strValue) std::string(std::move(other.variant_.strValue));
			break;

		case Kind::StringList:
			new (&variant_.strList) std::vector<std::string>(std::move(other.variant_.strList));
			break;

		case Kind::Bool:
			variant_.boolValue = other.variant_.boolValue;
			break;

		case Kind::Long:
			variant_.longValue = other.variant_.longValue;
			break;

		case Kind::Empty:
		default:
			break;
	}
}

DOCOPT_INLINE
docopt::value::~value()
{
	switch (kind_) {
		case Kind::String:
			variant_.strValue.~basic_string();
			break;

		case Kind::StringList:
			variant_.strList.~vector();
			break;

		case Kind::Empty:
		case Kind::Bool:
		case Kind::Long:
		default:
			// trivial dtor
			break;
	}
}

DOCOPT_INLINE
docopt::value& docopt::value::operator=(value const& other) {
	// make a copy and move from it; way easier.
	return *this = value{other};
}

DOCOPT_INLINE
docopt::value& docopt::value::operator=(value&& other) noexcept {
	// move of all the types involved is noexcept, so we dont have to worry about 
	// these two statements throwing, which gives us a consistency guarantee.
	this->~value();
	new (this) value(std::move(other));

	return *this;
}

DOCOPT_INLINE
docopt::Kind docopt::value::kind() const {
	return kind_;
}

docopt::value::operator bool() const {
	return kind_ != Kind::Empty;
}

DOCOPT_INLINE
bool docopt::value::isBool() const {
	return kind_==Kind::Bool;
}

DOCOPT_INLINE
bool docopt::value::isString() const {
	return kind_==Kind::String;
}

DOCOPT_INLINE
bool docopt::value::isLong() const {
	return kind_==Kind::Long;
}

DOCOPT_INLINE
bool docopt::value::isStringList() const {
	return kind_==Kind::StringList;
}

const char* docopt::value::kindAsString(Kind kind) {
	switch (kind) {
		case Kind::Empty: return "empty";
		case Kind::Bool: return "bool";
		case Kind::Long: return "long";
		case Kind::String: return "string";
		case Kind::StringList: return "string-list";
	}
	return "unknown";
}

void docopt::value::throwIfNotKind(Kind expected) const {
	if (kind_ == expected)
		return;

	std::string error = "Illegal cast to ";
	error += kindAsString(expected);
	error += "; type is actually ";
	error += kindAsString(kind_);
	throw std::runtime_error(std::move(error));
}

template <class T>
void hash_combine(std::size_t& seed, const T& v);

DOCOPT_INLINE
size_t docopt::value::hash() const noexcept
{
	switch (kind_) {
		case Kind::String:
			return std::hash<std::string>()(variant_.strValue);

		case Kind::StringList: {
			size_t seed = std::hash<size_t>()(variant_.strList.size());
			for(auto const& str : variant_.strList) {
				hash_combine(seed, str);
			}
			return seed;
		}

		case Kind::Bool:
			return std::hash<bool>()(variant_.boolValue);

		case Kind::Long:
			return std::hash<long>()(variant_.longValue);

		case Kind::Empty:
		default:
			return std::hash<void*>()(nullptr);
	}
}

DOCOPT_INLINE
bool docopt::value::asBool() const
{
	throwIfNotKind(Kind::Bool);
	return variant_.boolValue;
}

DOCOPT_INLINE
long docopt::value::asLong() const
{
	// Attempt to convert a string to a long
	if (kind_ == Kind::String) {
		const std::string& str = variant_.strValue;
		std::size_t pos;
		const long ret = stol(str, &pos); // Throws if it can't convert
		if (pos != str.length()) {
			// The string ended in non-digits.
			throw std::runtime_error( str + " contains non-numeric characters.");
		}
		return ret;
	}
	throwIfNotKind(Kind::Long);
	return variant_.longValue;
}

DOCOPT_INLINE
std::string const& docopt::value::asString() const
{
	throwIfNotKind(Kind::String);
	return variant_.strValue;
}

DOCOPT_INLINE
std::vector<std::string> const& docopt::value::asStringList() const
{
	throwIfNotKind(Kind::StringList);
	return variant_.strList;
}

DOCOPT_INLINE
bool docopt::operator==(docopt::value const& v1, docopt::value const& v2)
{
	if (v1.kind_ != v2.kind_)
		return false;
	
	switch (v1.kind_) {
		case Kind::String:
			return v1.variant_.strValue==v2.variant_.strValue;

		case Kind::StringList:
			return v1.variant_.strList==v2.variant_.strList;

		case Kind::Bool:
			return v1.variant_.boolValue==v2.variant_.boolValue;

		case Kind::Long:
			return v1.variant_.longValue==v2.variant_.longValue;

		case Kind::Empty:
		default:
			return true;
	}
}

DOCOPT_INLINE
bool operator!=(docopt::value const& v1, docopt::value const& v2)
{
	return !(v1 == v2);
}

DOCOPT_INLINE
docopt::value::value::Variant::Variant()
{
}

DOCOPT_INLINE
docopt::value::value::Variant::~Variant()
{
	/* do nothing; will be destroyed by ~value */
}
