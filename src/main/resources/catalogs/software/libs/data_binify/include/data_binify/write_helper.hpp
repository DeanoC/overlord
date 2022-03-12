#pragma once
#include "core/core.h"
#include "dbg/assert.h"
#include <vector>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
//#include <format>
#include "fmt/format.hpp"
#include "vfile/vfile.h"

namespace Binify {

class WriteHelper {
public:
	WriteHelper();

	// defaults
	template<typename T> auto setDefaultType() -> void {
		fmt::format_to(std::back_inserter(buffer), ".type {}\n", typeToString<T>());
	}

	// returns a string with the binify text (it is alive until WriteHelper is deleted)
	char const* c_str();

	// these write the DSL derivative at the current position
	void allowNan(bool yesno);
	void allowInfinity(bool yesno);
	void setAddressLength(int bits);
	int getAddressLength() { return addressLen; };


	// alignment functions
	void align(int i);
	void align();

	// label functions
	// Reserving a label puts the name into the system without writing it
	// default will make it the implicit label to use for size etc.
	void reserveLabel(std::string const &name, bool makeDefault = false);

	// writes the label itself at the current position (can reserve at the same time)
	// this causes the label to be set to the current position
	void writeLabel(std::string const &name,
									bool reserve = false,
									std::string const comment_ = "",
									bool noCommentEndStatement_ = true);
	// writes the label into the current position, this will cause position in label
	// to be used in an expression
	void useLabel(std::string const &name,
								std::string baseBlock = "",
								bool reserve = false,
								bool addFixup = true,
								std::string const comment_ = "",
								bool noCommentEndStatement_ = true);

	// constants
	// consts are a seperate variable namespace that are not mutable
	void setConstant(std::string const &name,
									 int64_t value,
									 std::string const comment_ = "",
									 bool noCommentEndStatement_ = true);
	std::string getConstant(std::string const &name);
	void setConstantToExpression(std::string const &name,
															 std::string const &exp,
															 std::string const comment_ = "",
															 bool noCommentEndStatement_ = true);

	// variables
	// variables are named things that can set to expression whilst parsing
	// poss 0 are not updated during pass 1, meaning that using them during pass 1
	// will get you the last value set during pass 0. This allows for counter/size etc.
	void setVariable(std::string const &name,
									 int64_t value,
									 bool pass0 = false,
									 std::string const comment_ = "",
									 bool noCommentEndStatement_ = true);
	void setVariableToExpression(std::string const &name,
															 std::string const &exp,
															 bool pass0 = false,
															 std::string const comment_ = "",
															 bool noCommentEndStatement_ = true);
	std::string getVariable(std::string const &name);
	void incrementVariable(std::string const &str_,
												 std::string const comment_ = "",
												 bool noCommentEndStatement_ = true);

	// enum and flags functions
	// enums are named types, each enum value belongs to a single enum and form
	// a set of constants that are then output into the binify text
	// this helps readibiltiy
	// flags are an extension of this that allow you to pass a bitwise flag set
	// but produce nice readable binify with the flags all given names.
	void addEnum(std::string const &name);
	void addEnumValue(std::string const &enum_name,
										std::string const &value_name,
										uint64_t value,
										std::string const comment_ = "",
										bool noCommentEndStatement_ = true);
	std::string getEnumValue(std::string const &name, std::string const &value_name);
	void writeEnum(std::string const &name,
								 std::string const &value_name,
								 std::string const comment_ = "",
								 bool noCommentEndStatement_ = true);
	void writeFlags(std::string const &name,
									uint64_t flags,
									std::string const comment_ = "",
									bool noCommentEndStatement_ = true);

	// string table functions
	// the string table handles all the management of strings lik4 copying, offset etc.
	// After fixup each string pointer will point to the correct portion of the file
	void addString(std::string_view str);  ///< adds it to the table and outputs a fixup
	std::string addStringToTable(std::string const &str);
	// sometimes its useful to have multiple string tables. this allows it by
	// setting the base the fixup will refer to
	void setStringTableBase(std::string const &label);

	// expression functions
	// an expression can be for integers + - | () and variable/constant number
	void writeExpression(std::string const &str_,
											 std::string const comment_ = "",
											 bool noCommentEndStatement_ = true) {
		fmt::format_to(std::back_inserter(buffer), "{}", str_.c_str());
		comment(comment_, noCommentEndStatement_);
	}

	template<typename type>
	void writeExpressionAs(std::string const &str_,
												 std::string const comment_ = "",
												 bool noCommentEndStatement_ = true) {
		fmt::format_to(std::back_inserter(buffer), "({}){}", typeToString<type>(), str_.c_str());
		comment(comment_, noCommentEndStatement_);
	}

	// misc functions
	// writes current position - defaultBlock into the file
	void sizeOfBlock(std::string const &name,
									 std::string const comment = "",
									 bool noCommentEndStatement_ = true);
	// adds a comment
	void comment(std::string const &comment, bool noCommentEndStatement_ = true);

	// writing functions
	// output a null pointer (addresslen) 0
	void writeNullPtr(std::string const comment = "",
										bool noCommentEndStatement_ = true); // outputs an address size 0 (without fixup of course!)

	// writes (addressLen)
	void writeAddressType(); ///< outputs a address type prefix
	void writeByteArray(std::vector<uint8_t> const &barray); ///< writes a byte array
	void writeByteArray(uint8_t const *bytes_, size_t size_); ///< writes a byte array

	// template single element write with optional comment
	template<typename T>
	void write(T i_, std::string const comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}", i_);
		comment(comment_);
	}

	void write(std::string const &str_, std::string const comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}", str_.c_str());
		comment(comment_);
	}

	// template 2 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, std::string const comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}, {}", i0_, i1_);
		comment(comment_);
	}

	// template 3 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, T i2_, std::string const comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}, {}, {}", i0_, i1_, i2_);
		comment(comment_);
	}

	// template 4 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, T i2_, T i3_, std::string const comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}, {}, {}, {}", i0_, i1_, i2_, i3_);
		comment(comment_);
	}

	// returns the binify type string for supported types (except strings)
	template<typename T> char const *const typeToString() const {
		if (std::is_signed<T>()) {
			if (typeid(T) == typeid(double)) {
				return "Double";
			}
			if (typeid(T) == typeid(float)) {
				return "Float";
			}

			switch (sizeof(T)) {
			case 1: return "s8";
			case 2: return "s16";
			case 4: return "s32";
			case 8: return "s64";
			default: assert(sizeof(T) == -1);
			}
		} else {
			switch (sizeof(T)) {
			case 1: return "u8";
			case 2: return "u16";
			case 4: return "u32";
			case 8: return "u64";
			default: assert(sizeof(T) == -1);
			}
		}
		return "unknown";
	}

	// template single element write as type with optional comment
	template<typename type, typename T>
	void writeAs(T i_, std::string const comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "({}) {}", typeToString<type>(), i_);
		comment(comment_);
	}

	template<typename type, typename T>
	void writeAs(T i0_, T i1_, std::string const comment_ = "") {
		auto typ = typeToString<type>();
		fmt::format_to(std::back_inserter(buffer), "({}) {}, ({}) {}",
									 typ, i0_, typ, i1_);
		comment(comment_);

	}

	template<typename type, typename T>
	void writeAs(T i0_, T i1_, T i2_, std::string const comment_ = "") {
		auto typ = typeToString<type>();
		fmt::format_to(std::back_inserter(buffer), "({}) {}, ({}) {}, ({}) {}",
									 typ, i0_, typ, i1_, typ, i2_
		);

		comment(comment_);
	}

	template<typename type, typename T>
	void writeAs(T i0_, T i1_, T i2_, T i3_, std::string const comment_ = "") {
		auto typ = typeToString<type>();
		fmt::format_to(std::back_inserter(buffer), "({}) {}, ({}) {}, ({}) {}, ({}) {}",
									 typ, i0_, typ, i1_, typ, i2_, typ, i3_
		);
		comment(comment_);
	}

	template<typename T>
	void writeSize(T i_, std::string const comment_ = "") {
		writeAddressType();
		fmt::format_to(std::back_inserter(buffer), "{}", i_);
		comment(comment_);
	}
	void finishStringTable();

	size_t getFixupCount() const { return fixups.size(); }
	std::string const& getFixup(size_t index) const { return fixups[index]; }
private:
	fmt::memory_buffer buffer;
	std::string outputString;

	std::string stringTableBase = "stringTable";

	void mergeStringTable(WriteHelper &other);
	void clearStringTable();
	std::string nameToLabel(std::string const &name);

	std::unordered_map<std::string, std::string> labelToStringTable;
	std::unordered_map<std::string, std::string> reverseStringTable;

	std::vector<std::string> fixups;
	std::unordered_set<std::string> labels;
	std::unordered_set<std::string> variables;
	std::unordered_set<std::string> constants;
	std::unordered_map<std::string, std::unordered_map<std::string, uint64_t>> enums;

	std::string defaultBlock;
	int addressLen = 64;

};

} // end namespace
