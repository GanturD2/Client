#ifndef __INC_SCRIPT_PARSER_H__
#define __INC_SCRIPT_PARSER_H__

#include <list>
#include <string>
#include "../UserInterface/Locale_inc.h"

namespace script
{
typedef struct SArgumet
{
	SArgumet(const std::string & c_stName, const std::string & c_stValue)
	{
		strName = c_stName;
		strValue = c_stValue;
	}
	SArgumet(SArgumet &&) = default;
	SArgumet & operator=(SArgumet &&) = delete;
	SArgumet(const SArgumet & c_arg)
	{
		strName = c_arg.strName;
		strValue = c_arg.strValue;
	}
	void operator=(const SArgumet & c_arg)
	{
		strName = c_arg.strName;
		strValue = c_arg.strValue;
	}
	std::string strName;
	std::string strValue;
} TArg;

typedef std::list<TArg> TArgList;

typedef struct SCmd
{
	std::string name;
	TArgList argList;

	SCmd() = default;
	SCmd & operator=(SCmd &&) = delete;
	SCmd(SCmd &&) = delete;
	SCmd(const SCmd & c_cmd)
	{
		name = c_cmd.name;
		argList = c_cmd.argList;
	}
	void operator=(const SCmd & c_cmd)
	{
		name = c_cmd.name;
		argList = c_cmd.argList;
	}
} TCmd;

class Group
{
public:
	Group();
	~Group();

public:
	/** ��Ʈ������ ���� ��ũ��Ʈ �׷��� �����.
			*
			* �����ϸ� GetError �޼ҵ�� Ȯ���� �� �ִ�.
			*
			* @param stSource �� ��Ʈ������ ���� �׷��� ����� ����.
			* @return ������ true, �����ϸ� false
			*/
	int Create(const std::string & stSource);

	/** ���ɾ �޴� �޼ҵ�
			*
			* @param cmd �����ÿ� �� ����ü�� ���ɾ ���� �ȴ�.
			* @return ���ɾ ���� �ִٸ� true, ���ٸ� false
			*/
	bool GetCmd(TCmd & cmd);

	/*
				���ɾ �������� �������� �ʴ´�.
			*/
	bool ReadCmd(TCmd & cmd);

	/** ������ ��� �޴� �޼ҵ�
			*
			* @return stError �� ������ ������ ��� �ȴ�.
			*/
	std::string & GetError();

private:
	void SetError(const char * c_pszError);
	bool GetArg(const char * c_arg_base, int arg_len, TArgList & argList);

	std::string m_stError;
	std::list<TCmd> m_cmdList;
};
} // namespace script

#endif