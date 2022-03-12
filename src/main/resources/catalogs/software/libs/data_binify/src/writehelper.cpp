#include "al2o3_platform/platform.h"
#include "data_binify/writehelper.hpp"


namespace Binify {

WriteHelper::WriteHelper()
{
	// system labels
	ReserveLabel("stringTable");
	ReserveLabel("stringTableEnd");
	ReserveLabel("chunks");
	ReserveLabel("chunksEnd");

	ReserveLabel("beginEnd");

	buffer.reserve(1024*1024); // reserve 1 MB output buffer
}

char const* WriteHelper::c_str() {
	char* tmp = (char*)MEMORY_TEMP_MALLOC(buffer.size()+1);
	memcpy(tmp, buffer.data(), buffer.size());
	tmp[buffer.size()] = 0;
	outputString = tinystl::string(tmp);
	MEMORY_TEMP_FREE(tmp);

	return outputString.c_str();
}

void WriteHelper::SetVariable(tinystl::string const &name,
															int64_t value,
															bool pass0,
															tinystl::string comment_,
															bool noCommentEndStatement_) {
	SetVariableToExpression(name, tinystl::to_string(value), pass0, comment_, noCommentEndStatement_);
}

void WriteHelper::SetVariableToExpression(tinystl::string const &name,
																					tinystl::string const &exp,
																					bool pass0,
																					tinystl::string comment_,
																					bool noCommentEndStatement_) {

	variables.insert(name);

	// pass 0 variables output the last set in pass 0, so work for counters
	if (pass0) {
		fmt::format_to(buffer, "*VAR_{}* {}", name.c_str(), exp.c_str());
	} else {
		fmt::format_to(buffer, "VAR_{} = {}", name.c_str(), exp.c_str());
	}
	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::SetConstant(tinystl::string const& name, int64_t value, tinystl::string const comment_, bool noCommentEndStatement_)
{
	SetConstantToExpression(name, tinystl::to_string(value), comment_, noCommentEndStatement_);
}

void WriteHelper::SetConstantToExpression(tinystl::string const& name, tinystl::string const& exp, tinystl::string const comment_, bool noCommentEndStatement_)
{
	assert(constants.find(name) == constants.end());
	constants.insert(name);
	fmt::format_to(buffer, "*CONST_{}* {}", name.c_str(), exp.c_str());
	Comment(comment_, noCommentEndStatement_);
}
tinystl::string WriteHelper::GetVariable(tinystl::string const& name)
{
	assert(variables.find(name) != variables.end());
	return " VAR_" + name;
}

tinystl::string WriteHelper::GetConstant(tinystl::string const& name)
{
	assert(constants.find(name) != constants.end());
	return " CONST_" + name;
}

void WriteHelper::IncrementVariable(tinystl::string const& str_, tinystl::string const comment_, bool noCommentEndStatement_)
{
	tinystl::string var = GetVariable(str_);
	SetVariableToExpression(str_, var + "+ 1");
	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::AddEnum(tinystl::string const& name)
{
	assert(enums.find(name) == enums.end());
	enums[name] = {};
}

void WriteHelper::AddEnumValue(tinystl::string const& name, tinystl::string const& value_name, uint64_t value, tinystl::string const comment_, bool noCommentEndStatement_)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	assert(e.find(value_name) == e.end());
	e[value_name] = value;
	SetConstant(name + "_" + value_name, value,comment_, noCommentEndStatement_);
}

tinystl::string WriteHelper::GetEnumValue(tinystl::string const& name, tinystl::string const& value_name)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	assert(e.find(value_name) != e.end());

	return GetConstant(name + "_" + value_name);
}

void WriteHelper::WriteEnum(tinystl::string const& name, tinystl::string const& value_name, tinystl::string const comment_, bool noCommentEndStatement_)
{
	fmt::format_to(buffer, "{}", GetEnumValue(name, value_name).c_str());
	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::WriteFlags(tinystl::string const& name, uint64_t flags, tinystl::string const comment_, bool noCommentEndStatement_)
{
	assert(enums.find(name) != enums.end());
	auto& e = enums[name];
	fmt::format_to(buffer, "0");
	for(auto const en : e)
	{
		if(flags & en.second)
		{
			fmt::format_to(buffer, " | {}", GetEnumValue(name, en.first).c_str());
		}
	}
	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::Comment(tinystl::string const& comment, bool noCommentEndStatement) {
	if (!comment.empty())
	{
		fmt::format_to(buffer, " // {}\n", comment.c_str());
	}
	else if(noCommentEndStatement)
	{
		fmt::format_to(buffer, "\n");
	}
}

void WriteHelper::AllowNan(bool yesno)
{
	fmt::format_to(buffer, ".allownan {}\n", (yesno ? "1" : "0"));
}

void WriteHelper::AllowInfinity(bool yesno)
{
	fmt::format_to(buffer, ".allowinfinity {}\n", (yesno ? "1" : "0"));
}

void WriteHelper::Align(int i)
{
	fmt::format_to(buffer, ".align {}\n", tinystl::to_string(i).c_str());
}

void WriteHelper::Align()
{
	Align(addressLen / 8);
}

void WriteHelper::ReserveLabel(tinystl::string const& name, bool makeDefault)
{
	tinystl::string label = name;
	assert(labels.find(label) == labels.end());
	labels.insert(label);

	if (makeDefault)
	{
		defaultBlock = label;
	}
}

void WriteHelper::WriteLabel(tinystl::string const& name, bool reserve, tinystl::string const comment_, bool noCommentEndStatement_)
{
	if (reserve)
	{
		ReserveLabel(name);
	}

	assert(labels.find(name) != labels.end());
	Align();
	fmt::format_to(buffer, "{}:", name.c_str());
	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::UseLabel(tinystl::string const& name, tinystl::string baseBlock, bool reserve, bool addFixup,  tinystl::string const comment_, bool noCommentEndStatement_)
{
	if (reserve)
	{
		ReserveLabel(name);
	}
	assert(labels.find(name) != labels.end());

	if (addFixup)
	{
		tinystl::string fixupLabel = "FIXUP_" + tinystl::to_string(fixups.size());
		fixups.push_back(fixupLabel);
		WriteLabel(fixupLabel, true);
	}

	if (baseBlock.empty())
	{
		baseBlock = defaultBlock;
	}
	assert(name != baseBlock);

	fmt::format_to(buffer, ".fixip {} - {}", name.c_str(), baseBlock.c_str());
	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::SetStringTableBase(tinystl::string const& label)
{
	stringTableBase = label;
}

void WriteHelper::AddString(tinystl::string_view str_)
{
	tinystl::string str = tinystl::string(str_);
	tinystl::string stringLabel = AddStringToTable(str);
	UseLabel(stringLabel, stringTableBase, false, true, str);
}

tinystl::string WriteHelper::AddStringToTable(tinystl::string const& str)
{
	tinystl::string stringLabel;
	// dedup strings
	if (reverseStringTable.find(str) == reverseStringTable.end())
	{
		stringLabel = "STR_" + NameToLabel(str);

		labelToStringTable[stringLabel] = str;
		reverseStringTable[str] = stringLabel;

		ReserveLabel(stringLabel);
	}
	else
	{
		stringLabel = reverseStringTable[str];
	}
	return stringLabel;
}

tinystl::string WriteHelper::NameToLabel(tinystl::string const& name)
{
	tinystl::string clean = name;
	for(auto& ch : clean)
	{
		if(!std::isdigit(ch) && !std::isalpha(ch))
		{
			ch = '_';
		}
	}

	return clean;
}

void WriteHelper::MergeStringTable(WriteHelper& other)
{
	for(auto const& en :other.labelToStringTable)
	{
		tinystl::string newLabel = AddStringToTable(en.second);
		assert(newLabel == en.first);
		assert(labelToStringTable.find(en.first) != labelToStringTable.end());
	}
}

void WriteHelper::FinishStringTable()
{
	Align();
	WriteLabel("stringTable");

	for(auto const& en : labelToStringTable)
	{
		WriteLabel(en.first);
		fmt::format_to(buffer, "\"{}\", 0\n", en.second.c_str());
	}

	WriteLabel("stringTableEnd");
}

void WriteHelper::SizeOfBlock(tinystl::string const& name, tinystl::string const comment_, bool noCommentEndStatement_)
{
	tinystl::string const nameend = name + "End";

	assert(labels.find(name) != labels.end());
	assert(labels.find(nameend) != labels.end());
	fmt::format_to(buffer, "{} - {}", nameend.c_str(), name.c_str());

	Comment(comment_, noCommentEndStatement_);
}

void WriteHelper::SetAddressLength(int bits)
{
	fmt::format_to(buffer, ".addresslen {}", bits);
	Comment("Using " + tinystl::to_string(bits) + " bits for addresses");
	addressLen = bits;
}

void WriteHelper::WriteNullPtr(tinystl::string const comment_, bool noCommentEndStatement_)
{
	WriteAddressType();
	fmt::format_to(buffer, "0");
	Comment(comment_, noCommentEndStatement_);
}
void WriteHelper::WriteAddressType()
{
	fmt::format_to(buffer, "(u{})", tinystl::to_string(addressLen).c_str());
}

void WriteHelper::WriteByteArray(tinystl::vector<uint8_t> const& barray)
{
	WriteByteArray(barray.data(), barray.size());
}

void WriteHelper::WriteByteArray(uint8_t const* bytes_, size_t size_)
{
	SetDefaultType<uint8_t>();
	if (size_ == 0)
	{
		fmt::format_to(buffer, "0\n");
	}
	else
	{
		for (size_t i = 0; i < size_ - 1; i++)
		{
			fmt::format_to(buffer, "{}{}",
										 tinystl::to_string(bytes_[i]).c_str(),
										 ((i % 80) == 79) ? "\n" : ", ");
		}

		// write last byte without ,
		fmt::format_to(buffer, "{}\n", tinystl::to_string(bytes_[size_ -1]).c_str());
	}

	SetDefaultType<uint32_t>();
}

} // end namespace