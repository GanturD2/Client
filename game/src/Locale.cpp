#include "stdafx.h"

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
ETypes GetArgType(std::type_index type)
{
	auto it = TypeMap.find(type);

	if (it != TypeMap.end())
		return it->second;

	return ETypes::type_none;
}

void WriteArguments(TEMP_BUFFER* pBuf, const ArgList* pArguments, uint16_t& wSize)
{
	for (auto it = pArguments->begin(); it != pArguments->end(); ++it)
	{
		const auto& Arg = *it;
		ETypes ArgType = GetArgType(Arg.type());

		pBuf->write(&ArgType, sizeof(ArgType));
		wSize += sizeof(ArgType);

		switch (ArgType)
		{
			case ETypes::type_short:
			{
				const auto& Value = std::any_cast<short>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_unsigned_short:
			{
				const auto& Value = std::any_cast<unsigned short>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_int:
			{
				const auto& Value = std::any_cast<int>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_unsigned_int:
			{
				const auto& Value = std::any_cast<unsigned int>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_long:
			{
				const auto& Value = std::any_cast<long>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;
			
			case ETypes::type_long_long:
			{
				const auto& Value = std::any_cast<long long>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_unsigned_long_long:
			{
				const auto& Value = std::any_cast<unsigned long long>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_bool:
			{
				const auto& Value = std::any_cast<bool>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_char:
			{
				const auto& Value = std::any_cast<char>(Arg);

				pBuf->write(&Value, sizeof(Value));
				wSize += sizeof(Value);
			} break;

			case ETypes::type_float:
			{
				const auto& Value = std::any_cast<float>(Arg);
				std::string FloatString = std::to_string(Value);
				std::size_t StringSize = FloatString.find(".") + 1 + 2; // 2 decimal places

				pBuf->write(&StringSize, sizeof(StringSize));
				pBuf->write(FloatString.c_str(), StringSize);
				wSize += sizeof(StringSize);
				wSize += StringSize;
			} break;

			case ETypes::type_double:
			{
				const auto& Value = std::any_cast<double>(Arg);
				std::string FloatString = std::to_string(Value);
				std::size_t StringSize = FloatString.find(".") + 1 + 2; // 2 decimal places

				pBuf->write(&StringSize, sizeof(StringSize));
				pBuf->write(FloatString.c_str(), StringSize);
				wSize += sizeof(StringSize);
				wSize += StringSize;
			} break;

			case ETypes::type_long_double:
			{
				const auto& Value = std::any_cast<long double>(Arg);
				std::string FloatString = std::to_string(Value);
				std::size_t StringSize = FloatString.find(".") + 1 + 2; // 2 decimal places

				pBuf->write(&StringSize, sizeof(StringSize));
				pBuf->write(FloatString.c_str(), StringSize);
				wSize += sizeof(StringSize);
				wSize += StringSize;
			} break;

			case ETypes::type_cstring:
			{
				const auto& Value = std::any_cast<const char*>(Arg);
				std::size_t StringSize = strlen(Value);

				pBuf->write(&StringSize, sizeof(StringSize));
				pBuf->write(Value, StringSize);
				wSize += sizeof(StringSize);
				wSize += StringSize;
			} break;

			case ETypes::type_string:
			{
				const auto& Value = std::any_cast<std::string>(Arg);
				std::size_t StringSize = Value.size();

				pBuf->write(&StringSize, sizeof(StringSize));
				pBuf->write(Value.c_str(), StringSize);
				wSize += sizeof(StringSize);
				wSize += StringSize;
			} break;

			default:
			{
				sys_err("Unknown variable type of name %s", Arg.type().name());
			} break;
		}
	}
}
#else
#ifdef ENABLE_GROWTH_PET_SYSTEM
# include "CsvReader.h"
#endif

typedef std::map< std::string, std::string > LocaleStringMapType;

LocaleStringMapType localeString;

#ifdef ENABLE_GROWTH_PET_SYSTEM
typedef std::map<uint32_t, std::string> LocalePetSkillMapType;
LocalePetSkillMapType localePetSkill;
#endif

int g_iUseLocale = 0;

void locale_add(const char** strings)
{
	LocaleStringMapType::const_iterator iter = localeString.find(strings[0]);

	if (iter == localeString.end())
	{
		localeString.insert(std::make_pair(strings[0], strings[1]));
	}
}

const char* locale_find(const char* string)
{
	if (0 == g_iUseLocale)
	{
		return (string);
	}

	LocaleStringMapType::const_iterator iter = localeString.find(string);

	if (iter == localeString.end())
	{
		static char s_line[1024] = "@0949";
		strlcpy(s_line + 5, string, sizeof(s_line) - 5);

		sys_err("LOCALE_ERROR: \"%s\";", string);
		return s_line;
	}

	return iter->second.c_str();
}

#ifdef ENABLE_GROWTH_PET_SYSTEM
const char* locale_pet_skill_find(uint32_t vnum)
{
	LocalePetSkillMapType::const_iterator iter = localePetSkill.find(vnum);

	if (iter == localePetSkill.end())
		return "NoName";

	return iter->second.c_str();
}

void locale_pet_skill_init(const char* filename)
{
	cCsvTable nameData;

	if (!nameData.Load(filename, '\t'))
	{
		fprintf(stderr, "%s couldn't be loaded or its format is incorrect.\n", filename);
		return;
	}

	nameData.Next();

	while (nameData.Next())
	{
		if (nameData.ColCount() < 2)
			continue;

		localePetSkill.insert(std::make_pair(atoi(nameData.AsStringByIndex(0)), nameData.AsStringByIndex(1)));
	}

	nameData.Destroy();
}
#endif

const char* quote_find_end(const char* string)
{
	const char* tmp = string;
	int         quote = 0;

	while (*tmp)
	{
		if (quote && *tmp == '\\' && *(tmp + 1))
		{
			switch (*(tmp + 1))
			{
			case '"':
				tmp += 2;
				continue;
			}
		}
		else if (*tmp == '"')
		{
			quote = !quote;
		}
		else if (!quote && *tmp == ';')
			return (tmp);

		tmp++;
	}

	return (nullptr);
}

char* locale_convert(const char* src, int len)
{
	const char* tmp;
	int		i, j;
	char* buf, * dest;
	int		start = 0;
	char	last_char = 0;

	if (!len)
		return nullptr;

	buf = new char[len + 1];

	for (j = i = 0, tmp = src, dest = buf; i < len; i++, tmp++)
	{
		if (*tmp == '"')
		{
			if (last_char != '\\')
				start = !start;
			else
				goto ENCODE;
		}
		else if (*tmp == ';')
		{
			if (last_char != '\\' && !start)
				break;
			else
				goto ENCODE;
		}
		else if (start)
		{
		ENCODE:
			if (*tmp == '\\' && *(tmp + 1) == 'n')
			{
				*(dest++) = '\n';
				tmp++;
				last_char = '\n';
			}
			else
			{
				*(dest++) = *tmp;
				last_char = *tmp;
			}

			j++;
		}
	}

	if (!j)
	{
		M2_DELETE_ARRAY(buf);
		return nullptr;
	}

	*dest = '\0';
	return (buf);
}

#define NUM_LOCALES 2

void locale_init(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	char* buf;

	if (!fp) return;

	fseek(fp, 0L, SEEK_END);
	int i = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	i++;

	buf = new char[i];

	memset(buf, 0, i);

	fread(buf, i - 1, sizeof(char), fp);

	fclose(fp);

	const char* tmp;
	const char* end;

	char* strings[NUM_LOCALES];

	if (!buf)
	{
		sys_err("locale_read: no file %s", filename);
		exit(1);
	}

	tmp = buf;

	do
	{
		for (i = 0; i < NUM_LOCALES; i++)
			strings[i] = nullptr;

		if (*tmp == '"')
		{
			for (i = 0; i < NUM_LOCALES; i++)
			{
				if (!(end = quote_find_end(tmp)))
					break;

				strings[i] = locale_convert(tmp, end - tmp);
				tmp = ++end;

				while (*tmp == '\n' || *tmp == '\r' || *tmp == ' ') tmp++;

				if (i + 1 == NUM_LOCALES)
					break;

				if (*tmp != '"')
				{
					sys_err("locale_init: invalid format filename %s", filename);
					break;
				}
			}

			if (strings[0] == nullptr || strings[1] == nullptr)
				break;

			locale_add((const char**)strings);

			for (i = 0; i < NUM_LOCALES; i++)
				if (strings[i])
					M2_DELETE_ARRAY(strings[i]);
		}
		else
		{
			tmp = strchr(tmp, '\n');

			if (tmp)
				tmp++;
		}
	} while (tmp && *tmp);

	M2_DELETE_ARRAY(buf);
}
#endif
