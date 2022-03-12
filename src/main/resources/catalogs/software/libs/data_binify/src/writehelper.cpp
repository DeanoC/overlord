#include "core/core.h"
#include "data_binify/write_helper.hpp"

namespace Binify {

WriteHelper::WriteHelper()
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
	outputString = std::string(buffer.data(), buffer.size());
	return outputString.c_str();
}

void WriteHelper::setVariable(std::string const &name,
															int64_t value,
															bool pass0,
															std::string comment_,
															bool noCommentEndStatement_) {
	setVariableToExpression(name, std::to_string(value), pass0, comment_, noCommentEndStatement_);
}

void WriteHelper::setVariableToExpression(std::string const &name,
																					std::string const &exp,
																					bool pass0,
																					std::string comment_,
																					bool noCommentEndStatement_) {

	variables.insert(name);

	// pass 0 variables output the last set in pass 0, so work for counters
	if (pass0) {
		fmt::format_to(std::back_inserter(buffer), "*VAR_{}* {}", name.c_str(), exp.c_str());
	} else {
		fmt::format_to(std::back_inserter(buffer), "VAR_{} = {}", name.c_str(), exp.c_str());
	}
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::setConstant(std::string const& name, int64_t value, std::string const comment_, bool noCommentEndStatement_)
{
	setConstantToExpression(name, std::to_string(value), comment_, noCommentEndStatement_);
}

void WriteHelper::setConstantToExpression(std::string const& name, std::string const& exp, std::string const comment_, bool noCommentEndStatement_)
{
	assert(constants.find(name) == constants.end());
	constants.insert(name);
	fmt::format_to(std::back_inserter(buffer), "*CONST_{}* {}", name.c_str(), exp.c_str());
	comment(comment_, noCommentEndStatement_);
}
std::string WriteHelper::getVariable(std::string const& name)
{
	assert(variables.find(name) != variables.end());
	return " VAR_" + name;
}

std::string WriteHelper::getConstant(std::string const& name)
{
	assert(constants.find(name) != constants.end());
	return " CONST_" + name;
}

void WriteHelper::incrementVariable(std::string const& str_, std::string const comment_, bool noCommentEndStatement_)
{
	std::string var = getVariable(str_);
	setVariableToExpression(str_, var + "+ 1");
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::addEnum(std::string const& name)
{
	assert(enums.find(name) == enums.end());
	enums[name] = {};
}

void WriteHelper::addEnumValue(std::string const& name, std::string const& value_name, uint64_t value, std::string const comment_, bool noCommentEndStatement_)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	assert(e.find(value_name) == e.end());
	e[value_name] = value;
	setConstant(name + "_" + value_name, value,comment_, noCommentEndStatement_);
}

std::string WriteHelper::getEnumValue(std::string const& name, std::string const& value_name)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	assert(e.find(value_name) != e.end());

	return getConstant(name + "_" + value_name);
}

void WriteHelper::writeEnum(std::string const& name, std::string const& value_name, std::string const comment_, bool noCommentEndStatement_)
{
	fmt::format_to(std::back_inserter(buffer), "{}", getEnumValue(name, value_name).c_str());
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::writeFlags(std::string const& name, uint64_t flags, std::string const comment_, bool noCommentEndStatement_)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	fmt::format_to(std::back_inserter(buffer), "0");
	for(auto const en : e)
	{
		if(flags & en.second)
		{
			fmt::format_to(std::back_inserter(buffer), " | {}", getEnumValue(name, en.first).c_str());
		}
	}
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::comment(std::string const& comment, bool noCommentEndStatement) {
	if (!comment.empty())
	{
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
	fmt::format_to(std::back_inserter(buffer), ".align {}\n", std::to_string(i).c_str());
}

void WriteHelper::align()
{
	align(addressLen / 8);
}

void WriteHelper::reserveLabel(std::string const& name, bool makeDefault)
{
	std::string label = name;
	assert(labels.find(label) == labels.end());
	labels.insert(label);

	if (makeDefault)
	{
		defaultBlock = label;
	}
}

void WriteHelper::writeLabel(std::string const& name, bool reserve, std::string const comment_, bool noCommentEndStatement_)
{
	if (reserve)
	{
		reserveLabel(name);
	}

	assert(labels.find(name) != labels.end());
	align();
	fmt::format_to(std::back_inserter(buffer), "{}:", name.c_str());
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::useLabel(std::string const& name, std::string baseBlock, bool reserve, bool addFixup,  std::string const comment_, bool noCommentEndStatement_)
{
	if (reserve)
	{
		reserveLabel(name);
	}
	assert(labels.find(name) != labels.end());

	if (addFixup)
	{
		std::string fixupLabel = "FIXUP_" + std::to_string(fixups.size());
		fixups.push_back(fixupLabel);
		writeLabel(fixupLabel, true);
	}

	if (baseBlock.empty())
	{
		baseBlock = defaultBlock;
	}
	assert(name != baseBlock);

	fmt::format_to(std::back_inserter(buffer), ".fixip {} - {}", name.c_str(), baseBlock.c_str());
	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::setStringTableBase(std::string const& label)
{
	stringTableBase = label;
}

void WriteHelper::addString(std::string_view str_)
{
	std::string str = std::string(str_);
	std::string stringLabel = addStringToTable(str);
	useLabel(stringLabel, stringTableBase, false, true, str);
}

std::string WriteHelper::addStringToTable(std::string const& str)
{
	std::string stringLabel;
	// dedup strings
	if (reverseStringTable.find(str) == reverseStringTable.end())
	{
		stringLabel = "STR_" + nameToLabel(str);

		labelToStringTable[stringLabel] = str;
		reverseStringTable[str] = stringLabel;

		reserveLabel(stringLabel);
	}
	else
	{
		stringLabel = reverseStringTable[str];
	}
	return stringLabel;
}

std::string WriteHelper::nameToLabel(std::string const& name)
{
	std::string clean = name;
	for(auto& ch : clean)
	{
		if(!std::isdigit(ch) && !std::isalpha(ch))
		{
			ch = '_';
		}
	}

	return clean;
}

void WriteHelper::mergeStringTable(WriteHelper& other)
{
	for(auto const& en :other.labelToStringTable)
	{
		std::string newLabel = addStringToTable(en.second);
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

void WriteHelper::sizeOfBlock(std::string const& name, std::string const comment_, bool noCommentEndStatement_)
{
	std::string const nameend = name + "End";

	assert(labels.find(name) != labels.end());
	assert(labels.find(nameend) != labels.end());
	fmt::format_to(std::back_inserter(buffer), "{} - {}", nameend.c_str(), name.c_str());

	comment(comment_, noCommentEndStatement_);
}

void WriteHelper::setAddressLength(int bits)
{
	fmt::format_to(std::back_inserter(buffer), ".addresslen {}", bits);
	comment("Using " + std::to_string(bits) + " bits for addresses");
	addressLen = bits;
}


void WriteHelper::writeNullPtr(std::string const comment_, bool noCommentEndStatement_)
{
	writeAddressType();
	fmt::format_to(std::back_inserter(buffer), "0");
	comment(comment_, noCommentEndStatement_);
}
void WriteHelper::writeAddressType()
{
	fmt::format_to(std::back_inserter(buffer), "(u{})", std::to_string(addressLen).c_str());
}

void WriteHelper::writeByteArray(std::vector<uint8_t> const& barray)
{
	writeByteArray(barray.data(), barray.size());
}

void WriteHelper::writeByteArray(uint8_t const* bytes_, size_t size_)
{
	setDefaultType<uint8_t>();
	if (size_ == 0)
	{
		fmt::format_to(std::back_inserter(buffer), "0\n");
	}
	else
	{
		for (size_t i = 0; i < size_ - 1; i++)
		{
			fmt::format_to(std::back_inserter(buffer), "{}{}",
										 std::to_string(bytes_[i]).c_str(),
										 ((i % 80) == 79) ? "\n" : ", ");
		}

		// write last byte without ,
		fmt::format_to(std::back_inserter(buffer), "{}\n", std::to_string(bytes_[size_ -1]).c_str());
	}

	setDefaultType<uint32_t>();
}

} // end namespace