#include "core/core.h"
#include "data_binify/write_helper.hpp"

namespace Binify {

WriteHelper::WriteHelper(Memory_Allocator* allocator_) :
	allocator(allocator_),
	outputString(allocator),
	stringTableBase("stringTable", allocator),
	labelToStringTable(allocator),
	reverseStringTable(allocator),
	fixups(allocator),
	labels(allocator),
	variables(allocator),
	constants(allocator),
	enumValueVector(allocator),
	enums(allocator),
	defaultBlock(allocator)
{
	// system labels
	reserveLabel("stringTable");
	reserveLabel("stringTableEnd");
	reserveLabel("chunks");
	reserveLabel("chunksEnd");

	reserveLabel("beginEnd");

	buffer.reserve(1024*1024); // reserve 1 MB output buffer
}

char const* WriteHelper::c_str() {
	outputString = tiny_stl::string(buffer.data(), buffer.size(), allocator);
	return outputString.c_str();
}

void WriteHelper::setVariable(tiny_stl::string_view const & name_,
															int64_t value_,
															bool pass0_,
															tiny_stl::string_view const & comment_,
															bool noCommentEndStatement_) {
	MEMORY_STACK_ALLOCATOR(_, 1024);
	setVariableToExpression(name_, tiny_stl::to_string(value_, _), pass0_, comment_, noCommentEndStatement_);
}

void WriteHelper::setVariableToExpression(tiny_stl::string_view const & name_,
                                          tiny_stl::string_view const & exp_,
																					bool pass0_,
																					tiny_stl::string_view const & comment_,
																					bool noCommentEndStatement_) {
	MEMORY_STACK_ALLOCATOR(_, 1024);

	tiny_stl::string const name { name_, allocator};
	tiny_stl::string const exp { exp_, _ };

	variables.insert(name);

	// pass 0 variables output the last set in pass 0, so work for counters
	if (pass0_) {
		fmt::format_to(std::back_inserter(buffer), "*VAR_{}* {}", name.c_str(), exp.c_str());
	} else {
		fmt::format_to(std::back_inserter(buffer), "VAR_{} = {}", name.c_str(), exp.c_str());
	}
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::setConstant(tiny_stl::string_view const & name_,
															int64_t value_,
															tiny_stl::string_view const & comment_,
															bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	setConstantToExpression(name_, tiny_stl::to_string(value_,_), comment_, noCommentEndStatement_);
}

void WriteHelper::setConstantToExpression(tiny_stl::string_view const & name_,
																					tiny_stl::string_view const & exp_,
																					tiny_stl::string_view const & comment_,
																					bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, allocator);
	tiny_stl::string const exp(exp_, _);
	assert(constants.find(name) == constants.end());
	constants.insert(name);
	fmt::format_to(std::back_inserter(buffer), "*CONST_{}* {}", name.c_str(), exp.c_str());
	comment(comment_, noCommentEndStatement_);
}
tiny_stl::string WriteHelper::getVariable(tiny_stl::string_view const & name_) const
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, _);

	assert(variables.find(name) != variables.end());
	return tiny_stl::string(" VAR_", allocator) + name;
}

tiny_stl::string WriteHelper::getConstant(tiny_stl::string_view const & name_) const
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, _);

	assert(constants.find(name) != constants.end());
	return tiny_stl::string(" CONST_", allocator) + name;
}

void WriteHelper::incrementVariable(tiny_stl::string_view const & str_,
																		uint32_t adder,
																		tiny_stl::string_view const & comment_,
																		bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);

	tiny_stl::string var = getVariable(str_);
	setVariableToExpression(str_, var + " + " + tiny_stl::to_string(adder,_));
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::addEnum(tiny_stl::string_view const & name_)
{
	tiny_stl::string const name(name_, allocator);

	assert(enums.find(name) == enums.end());
	enumValueVector.emplace_back( enumValue_t{allocator } );
	enums.insert( tiny_stl::pair(name, enumValueVector.size()-1) );
}

void WriteHelper::addEnumValue(tiny_stl::string_view const & name_,
                               tiny_stl::string_view const & value_name_,
															 int64_t value,
															 tiny_stl::string_view const & comment_,
															 bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, _);
	tiny_stl::string const value_name(value_name_, allocator);

	assert(enums.find(name) != enums.end());
	size_t index = enums[name];
	auto& e = enumValueVector[index];

	assert(e.find(value_name) == e.end());
	e[value_name] = value;
	setConstant(name + "_" + value_name, value, comment_, noCommentEndStatement_);
}

tiny_stl::string WriteHelper::getEnumValue(tiny_stl::string_view const & name_,
																					tiny_stl::string_view const & value_name_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, _);
	tiny_stl::string const value_name(value_name_, _);

	assert(enums.find(name) != enums.end());
	size_t index = enums[name];
	auto& e = enumValueVector[index];
	assert(e.find(value_name) != e.end());

	return getConstant(name + "_" + value_name);
}

void WriteHelper::writeEnum(tiny_stl::string_view const & name_,
                            tiny_stl::string_view const & value_name_,
                            tiny_stl::string_view const & comment_,
														bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, _);
	tiny_stl::string const value_name(value_name_, _);

	fmt::format_to(std::back_inserter(buffer), "{}", getEnumValue(name, value_name).c_str());
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::writeFlags(tiny_stl::string_view const & name_,
														 uint64_t flags_,
														 tiny_stl::string_view const & comment_,
														 bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string const name(name_, _);

	assert(enums.find(name) != enums.end());
	size_t index = enums[name];
	auto& e = enumValueVector[index];
	fmt::format_to(std::back_inserter(buffer), "0");
	for(auto const en : e)
	{
		if(flags_ & en.second)
		{
			fmt::format_to(std::back_inserter(buffer), " | {}", getEnumValue(name, en.first).c_str());
		}
	}
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::comment(tiny_stl::string_view const & comment_,
													bool noCommentEndStatement) {

	if (!comment_.empty())
	{
		MEMORY_STACK_ALLOCATOR(_, 16 * 1024);
		tiny_stl::string const comment(comment_, _);
		fmt::format_to(std::back_inserter(buffer), " // {}\n", comment.c_str());
	}
	else if(noCommentEndStatement)
	{
		fmt::format_to(std::back_inserter(buffer), "\n");
	}
}

void WriteHelper::allowNan(bool yesno)
{
	fmt::format_to(std::back_inserter(buffer), ".allownan {}\n", (yesno ? "1" : "0"));
}

void WriteHelper::allowInfinity(bool yesno)
{
	fmt::format_to(std::back_inserter(buffer), ".allowinfinity {}\n", (yesno ? "1" : "0"));
}

void WriteHelper::align(int i)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	fmt::format_to(std::back_inserter(buffer), ".align {}\n", tiny_stl::to_string(i,_).c_str());
}

void WriteHelper::align()
{
	align(addressLen / 8);
}

void WriteHelper::reserveLabel(tiny_stl::string_view const &name_, bool makeDefault)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string label(name_, allocator);
	assert(labels.find(label) == labels.end());
	labels.insert(label);

	if (makeDefault)
	{
		defaultBlock = label;
	}
}

void WriteHelper::writeLabel(tiny_stl::string_view const &name_,
                             bool reserve_,
                             tiny_stl::string_view const & comment_,
                             bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	if (reserve_) reserveLabel(name_);

	tiny_stl::string name(name_, allocator);

	assert(labels.find(name) != labels.end());
	align();
	fmt::format_to(std::back_inserter(buffer), "{}:", name.c_str());
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::useLabel(tiny_stl::string_view const & name_,
                           tiny_stl::string_view const & baseBlock_,
													 bool reserve_,
													 bool addFixup_,
													 tiny_stl::string_view const & comment_,
													 bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	if (reserve_) reserveLabel(name_);

	tiny_stl::string name(name_, _);
	assert(labels.find(name) != labels.end());

	if (addFixup_)
	{
		tiny_stl::string fixupLabel(allocator);
		fmt::format_to(std::back_inserter(fixupLabel), "FIXUP_{}", fixups.size());
		fixups.emplace_back(fixupLabel);
		writeLabel(fixupLabel, true);
	}

	tiny_stl::string baseBlock(baseBlock_, _);
	if (baseBlock_.empty()) baseBlock = defaultBlock;
	assert(name != baseBlock);

	fmt::format_to(std::back_inserter(buffer), ".fixup {} - {}", name.c_str(), baseBlock.c_str());
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::setStringTableBase(tiny_stl::string_view const & label_)
{
	stringTableBase = tiny_stl::string(label_, allocator);
}

void WriteHelper::addString(tiny_stl::string_view const & str_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string str = tiny_stl::string(str_, _);
	tiny_stl::string stringLabel = addStringToTable(str);
	useLabel(stringLabel, stringTableBase, false, true, str);
}

tiny_stl::string WriteHelper::addStringToTable(tiny_stl::string_view const & str_)
{
	tiny_stl::string str = tiny_stl::string(str_, allocator);
	// dedup strings
	if (reverseStringTable.find(str) == reverseStringTable.end())
	{
		tiny_stl::string stringLabel { "STR_" + nameToLabel(str), allocator };

		labelToStringTable.insert(tiny_stl::pair(stringLabel,str));
		reverseStringTable.insert(tiny_stl::pair(str,stringLabel));

		reserveLabel(stringLabel);
		return stringLabel;
	}
	else
	{
		tiny_stl::string stringLabel = reverseStringTable.find(str)->second;
		return stringLabel;
	}
}

tiny_stl::string WriteHelper::nameToLabel(tiny_stl::string_view const & name_)
{
	tiny_stl::string clean { name_, allocator };
	for(auto& ch : clean)
	{
		if(!isdigit(ch) && !isalpha(ch)) ch = '_';
	}

	return clean;
}

void WriteHelper::clearStringTable() {
	labelToStringTable.clear();
	reverseStringTable.clear();
}

void WriteHelper::mergeStringTable(WriteHelper& other_)
{
	for(auto const& en :other_.labelToStringTable)
	{
		tiny_stl::string newLabel = addStringToTable(en.second);
		assert(newLabel == en.first);
		assert(labelToStringTable.find(en.first) != labelToStringTable.end());
	}
}

void WriteHelper::finishStringTable()
{
	align();
	writeLabel("stringTable");

	for(auto const& en : labelToStringTable)
	{
		writeLabel(en.first);
		fmt::format_to(std::back_inserter(buffer), "\"{}\", 0\n", en.second.c_str());
	}

	writeLabel("stringTableEnd");
}

void WriteHelper::sizeOfBlock(tiny_stl::string_view const & name_, tiny_stl::string_view const & comment_, bool noCommentEndStatement_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	tiny_stl::string name(name_, allocator);
	tiny_stl::string const nameend = name + "End";

	assert(labels.find(name) != labels.end());
	assert(labels.find(nameend) != labels.end());
	fmt::format_to(std::back_inserter(buffer), "{} - {}", nameend.c_str(), name.c_str());

	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::setAddressLength(int bits_)
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	fmt::format_to(std::back_inserter(buffer), ".addresslen {}", bits_);
	comment(tiny_stl::string("Using ", _) + tiny_stl::to_string(bits_, _) + " bits for addresses");
	addressLen = bits_;
}


void WriteHelper::writeNullPtr(tiny_stl::string_view const & comment_, bool noCommentEndStatement_)
{
	writeAddressType();
	fmt::format_to(std::back_inserter(buffer), "0");
	comment(comment_, noCommentEndStatement_);
}
void WriteHelper::writeAddressType()
{
	MEMORY_STACK_ALLOCATOR(_, 1024);
	fmt::format_to(std::back_inserter(buffer), "(u{})", tiny_stl::to_string(addressLen,_).c_str());
}

void WriteHelper::writeByteArray(uint8_t const* bytes_, size_t size_) {
	writeByteArray(Utils::Slice<uint8_t const>{ .data = bytes_, .size = size_ });
}

void WriteHelper::writeByteArray(Utils::Slice<uint8_t const> slice_) {
	setDefaultType<uint8_t>();
	if (slice_.size == 0) fmt::format_to(std::back_inserter(buffer), "0\n");
	else
	{
		for (size_t i = 0; i < slice_.size - 1; i++)
		{
			MEMORY_STACK_ALLOCATOR(_, 32);
			fmt::format_to(std::back_inserter(buffer), "{}{}",
										 tiny_stl::to_string(slice_.data[i], _).c_str(),
										 ((i % 80) == 79) ? "\n" : ", ");
		}

		// write last byte without ,
		MEMORY_STACK_ALLOCATOR(_, 32);
		fmt::format_to(std::back_inserter(buffer), "{}\n", tiny_stl::to_string(slice_.data[slice_.size - 1], _).c_str());
	}

	setDefaultType<uint32_t>();
}

} // end namespace