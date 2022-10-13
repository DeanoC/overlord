#pragma once
#include "core/core.h"
#include "dbg/assert.h"
#include "tiny_stl/vector.hpp"
#include "tiny_stl/string.hpp"
#include "tiny_stl/string_view.hpp"
#include "tiny_stl/unordered_set.hpp"
#include "tiny_stl/unordered_map.hpp"
#include "fmt/format.hpp"
#include "vfile/vfile.h"
#include "utils/slice.hpp"
namespace Binify {

class WriteHelper {
public:
	WriteHelper(Memory_Allocator* allocator_);

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
	WARN_UNUSED_RESULT int getAddressLength() const { return addressLen; };


	// alignment functions
	void align(int i);
	void align();

	// label functions
	// Reserving a label puts the name into the system without writing it
	// default will make it the implicit label to use for size etc.
	void reserveLabel(tiny_stl::string_view const &name_, bool makeDefault_ = false);

	// assign the label itself at the current position (can reserve at the same time)
	// this causes the label to be set to the current position
	void assignLabel(tiny_stl::string_view const &name_,
									bool reserve_ = false,
									tiny_stl::string_view const & comment_ = "",
									bool noCommentEndStatement_ = true);
	// writes the label into the current position, this will cause position in label
	// to be used in an expression
	void useLabel(tiny_stl::string_view const & name_,
	              tiny_stl::string_view const & baseBlock_ = "",
								bool reserve_ = false,
								bool addFixup_ = true,
								tiny_stl::string_view const & comment_ = "",
								bool noCommentEndStatement_ = true);

	// these use the address itself as the label name, to make pointer easier to include and fixup
	void useAddressAsLabel(void const * address,
	              tiny_stl::string_view const & baseBlock_ = "",
	              bool reserve_ = false,
	              bool addFixup_ = true,
	              tiny_stl::string_view const & comment_ = "",
	              bool noCommentEndStatement_ = true);
	void assignAddressAsLabel(void const * address,
												 bool reserve_ = false,
	                       tiny_stl::string_view const & comment_ = "",
	                       bool noCommentEndStatement_ = true);

	// constants
	// consts are a separate variable namespace that are not mutable
	void setConstant(tiny_stl::string_view const &name_,
									 int64_t value_,
									 tiny_stl::string_view const & comment_  = "",
									 bool noCommentEndStatement_ = true);

	WARN_UNUSED_RESULT tiny_stl::string getConstant(tiny_stl::string_view const & name_) const;

	void setConstantToExpression(tiny_stl::string_view const & name_,
	                             tiny_stl::string_view const & exp_,
	                             tiny_stl::string_view const & comment_ = "",
															 bool noCommentEndStatement_ = true);

	// variables are named things that can set to an expression whilst parsing
	// pass 0 vars are not updated during pass 1, meaning that using them during pass 1
	// will get you the last value set during pass 0. This allows for counter/size etc.
	void setVariable(tiny_stl::string_view const & name_,
									 int64_t value_,
									 bool pass0_ = false,
									 tiny_stl::string_view const & comment_ = "",
									 bool noCommentEndStatement_ = true);

	void setVariableToExpression(tiny_stl::string_view const & name_,
	                             tiny_stl::string_view const & exp_,
															 bool pass0_ = false,
															 tiny_stl::string_view const & comment_ = "",
															 bool noCommentEndStatement_ = true);

	WARN_UNUSED_RESULT tiny_stl::string getVariable(tiny_stl::string_view const & name_) const;

	void writeVariable(tiny_stl::string_view const & name_,
										 tiny_stl::string_view const & comment_ = "",
	                   bool noCommentEndStatement_ = true);

	template<typename type>
	void writeVariableAs(tiny_stl::string_view const & name_,
	                     tiny_stl::string_view const & comment_ = "",
	                     bool noCommentEndStatement_ = true) {
		fmt::format_to(std::back_inserter(buffer), "({})", typeToString<type>());
		writeVariable(name_, comment_, noCommentEndStatement_);

	}

	void incrementVariable(tiny_stl::string_view const & name_,
	                       uint32_t adder_ = 1,
	                       tiny_stl::string_view const & comment_ = "",
												 bool noCommentEndStatement_ = true);

	// enum and flags functions
	// enums are named types, each enum value belongs to a single enum and form
	// a set of constants that are then output into the binify text
	// this helps readability
	// flags are an extension of this that allow you to pass a bitwise flag set
	// but produce nice readable binify with the flags all given names.
	void addEnum(tiny_stl::string_view const & name_);

	void addEnumValue(tiny_stl::string_view const & name_,
	                  tiny_stl::string_view const & value_name_,
										int64_t value_,
										tiny_stl::string_view const & comment_ = "",
										bool noCommentEndStatement_ = true);

	tiny_stl::string getEnumValue(tiny_stl::string_view const & name_, tiny_stl::string_view const & value_name);

	void writeEnum(tiny_stl::string_view const & name_,
	               tiny_stl::string_view const & value_name_,
	               tiny_stl::string_view const & comment_ = "",
								 bool noCommentEndStatement_ = true);
	void writeEnum(tiny_stl::string_view const & name_,
	               uint64_t value_,
	               tiny_stl::string_view const & comment_ = "",
	               bool noCommentEndStatement_ = true);

	template<typename type>
	void writeEnumAs(tiny_stl::string_view const & name_,
	                 tiny_stl::string_view const & value_name_,
	                 tiny_stl::string_view const & comment_ = "",
	                 bool noCommentEndStatement_ = true)
	{
		fmt::format_to(std::back_inserter(buffer), "({})", typeToString<type>());
		writeEnum(name_, value_name_, comment_, noCommentEndStatement_);
	}
		template<typename type>
		void writeEnumAs(tiny_stl::string_view const & name_,
		                 uint64_t value_,
		                 tiny_stl::string_view const & comment_ = "",
		                 bool noCommentEndStatement_ = true)
		{
			fmt::format_to(std::back_inserter(buffer), "({})", typeToString<type>());
			writeEnum(name_, value_, comment_, noCommentEndStatement_);
		}

	void writeFlags(tiny_stl::string_view const & name_,
									uint64_t flags_,
									tiny_stl::string_view const & comment_ = "",
									bool noCommentEndStatement_ = true);

	template<typename type>
	void writeFlagsAs(tiny_stl::string_view const & name_,
	                uint64_t flags_,
	                tiny_stl::string_view const & comment_ = "",
	                bool noCommentEndStatement_ = true)
	{
		fmt::format_to(std::back_inserter(buffer), "({})", typeToString<type>());
		writeFlags(name_, flags_, comment_, noCommentEndStatement_);
	}

	// string table functions
	// the string table handles all the management of strings lik4 copying, offset etc.
	// After fixup each string pointer will point to the correct portion of the file
	void addString(tiny_stl::string_view const & str_, bool addFixup_ = true);  ///< adds it to the table and outputs a fixup

	template<typename type>
	void addStringAs(tiny_stl::string_view const & str_, bool addFixup_ = true)
	{
		fmt::format_to(std::back_inserter(buffer), "({})", typeToString<type>());
		addString(str_, addFixup_);
	}

	tiny_stl::string addStringToTable(tiny_stl::string_view const & str_);

	// sometimes its useful to have multiple string tables. this allows it by
	// setting the base the fixup will refer to
	void setStringTableBase(tiny_stl::string_view const & label_);

	// expression functions
	// an expression can be for integers + - | () and variable/constant number
	void writeExpression(tiny_stl::string const & str_,
	                     tiny_stl::string_view const & comment_ = "",
											 bool noCommentEndStatement_ = true) {
		fmt::format_to(std::back_inserter(buffer), "{}", str_.c_str());
		comment(comment_, noCommentEndStatement_);
	}

	template<typename type>
	void writeExpressionAs(tiny_stl::string const & str_,
	                       tiny_stl::string_view const & comment_ = "",
												 bool noCommentEndStatement_ = true) {
		fmt::format_to(std::back_inserter(buffer), "({}){}", typeToString<type>(), str_.c_str());
		comment(comment_, noCommentEndStatement_);
	}

	// scope functions
	void pushScope() { scope += "_"; }
	void popScope() { scope.pop_back(); }

	// misc functions
	// writes current position - defaultBlock into the file
	void sizeOfBlock(tiny_stl::string_view const & name,
	                 tiny_stl::string_view const & comment_ = "",
									 bool noCommentEndStatement_ = true);
	// adds a comment
	void comment(tiny_stl::string_view const & comment_, bool noCommentEndStatement_ = true);

	// writing functions
	// output a null pointer (addresslen) 0
	void writeNullPtr(tiny_stl::string_view const & comment = "",
										bool noCommentEndStatement_ = true); // outputs an address size 0 (without fixup of course!)

	// writes (addressLen)
	void writeAddressType(); ///< outputs a address type prefix

	void writeByteArray(Utils::Slice<uint8_t const> slice_); // write a byte slice
	void writeByteArray(uint8_t const *bytes_, size_t size_); ///< writes a byte array

	// template single element write with optional comment
	template<typename T>
	void write(T i_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}", i_);
		comment(comment_);
	}

	void write(tiny_stl::string_view const & str_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}", str_.data());
		comment(comment_);
	}

	// template 2 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}, {}", i0_, i1_);
		comment(comment_);
	}

	// template 3 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, T i2_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}, {}, {}", i0_, i1_, i2_);
		comment(comment_);
	}

	// template 4 element write with optional comment
	template<typename T>
	void write(T i0_, T i1_, T i2_, T i3_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}, {}, {}, {}", i0_, i1_, i2_, i3_);
		comment(comment_);
	}

	// returns the binify type string for supported types (except strings)
	template<typename T>
	WARN_UNUSED_RESULT CONST_EXPR char const *typeToString() const {
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

	template<typename type>
	void sizeOfBlockAs(tiny_stl::string_view const & name_, tiny_stl::string_view const & comment_ = "", bool noCommentEndStatement_ = true)
	{
		tiny_stl::string const name { name_, allocator};
		tiny_stl::string const nameend { name + tiny_stl::string("End",allocator) };

		assert(labels.find(name) != labels.end());
		assert(labels.find(nameend) != labels.end());
		fmt::format_to(std::back_inserter(buffer), "({}) ({} - {})", typeToString<type>(), nameend.c_str(), name.c_str());

		comment(comment_, noCommentEndStatement_);
	}

	// template single element write as type with optional comment
	template<typename type>
	void useLabelAs(tiny_stl::string_view const & name_,
	                tiny_stl::string_view const & baseBlock_ = "",
	                bool reserve_ = false,
	                bool addFixup_ = true,
	                tiny_stl::string_view const & comment_ = "",
	                bool noCommentEndStatement_ = true) {
		fmt::format_to(std::back_inserter(buffer), "({}) ", typeToString<type>());
		useLabel(name_, baseBlock_, reserve_, addFixup_, comment_, noCommentEndStatement_);
	}

	// template single element write as type with optional comment
	template<typename type, typename T>
	void writeAs(T i_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "({}) {}", typeToString<type>(), i_);
		comment(comment_);
	}

	template<typename type, typename T>
	void writeAs(T i0_, T i1_, tiny_stl::string_view const & comment_ = "") {
		auto typ = typeToString<type>();
		fmt::format_to(std::back_inserter(buffer), "({}) {}, ({}) {}",
									 typ, i0_, typ, i1_);
		comment(comment_);

	}

	template<typename type, typename T>
	void writeAs(T i0_, T i1_, T i2_, tiny_stl::string_view const & comment_ = "") {
		auto typ = typeToString<type>();
		fmt::format_to(std::back_inserter(buffer), "({}) {}, ({}) {}, ({}) {}",
									 typ, i0_, typ, i1_, typ, i2_
		);

		comment(comment_);
	}

	template<typename type, typename T>
	void writeAs(T i0_, T i1_, T i2_, T i3_, tiny_stl::string_view const & comment_ = "") {
		auto typ = typeToString<type>();
		fmt::format_to(std::back_inserter(buffer), "({}) {}, ({}) {}, ({}) {}, ({}) {}",
									 typ, i0_, typ, i1_, typ, i2_, typ, i3_
		);
		comment(comment_);
	}

	template<typename T>
	void writeSize(T i_, tiny_stl::string_view const & comment_ = "") {
		fmt::format_to(std::back_inserter(buffer), "{}", i_);
		comment(comment_);
	}

	void finishStringTable();

	WARN_UNUSED_RESULT size_t getFixupCount() const { return fixups.size(); }
	WARN_UNUSED_RESULT tiny_stl::string const & getFixup(size_t index) const { return fixups[index]; }

private:
	void clearStringTable();
	void mergeStringTable(WriteHelper &other);
	tiny_stl::string nameToLabel(tiny_stl::string_view const &name_);

	Memory_Allocator* allocator;
	fmt::memory_buffer buffer;
	tiny_stl::string outputString;
	tiny_stl::string stringTableBase;

	tiny_stl::unordered_map<tiny_stl::string, tiny_stl::string> labelToStringTable;
	tiny_stl::unordered_map<tiny_stl::string, tiny_stl::string> reverseStringTable;

	tiny_stl::vector<tiny_stl::string> fixups;
	tiny_stl::unordered_set<tiny_stl::string> labels;
	tiny_stl::unordered_set<tiny_stl::string> variables;
	tiny_stl::unordered_set<tiny_stl::string> constants;

	typedef tiny_stl::unordered_map<tiny_stl::string, uint64_t> enumValue_t;
	tiny_stl::vector<enumValue_t> enumValueVector;
	tiny_stl::unordered_map<tiny_stl::string, size_t> enums;

	tiny_stl::string scope;
	tiny_stl::string defaultBlock;
	int addressLen = 64;

};

} // end namespace
