#pragma once
#include "Packet.h"
#ifdef ENABLE_CUBE_RENEWAL
class CPythonCubeRenewal : public CSingleton<CPythonCubeRenewal>
{
public:
	struct CUBE_VALUE
	{
		int32_t	vnum;
		int32_t		count;
		bool operator == (const CUBE_VALUE& b)
		{
			return (this->count == b.count) && (this->vnum == b.vnum);
		}
	};

	struct CUBE_DATA
	{
		std::vector<int32_t> npc_vnum;

		std::vector<CUBE_VALUE> item;
		std::vector<CUBE_VALUE> reward;
		int percent;
		int32_t gold;
		int32_t gem;
		bool allow_copy;
		std::string category;
		int not_remove;
		int set_value;
		CUBE_DATA() : set_value(0), gem(0) {}
	};

public:
	typedef std::vector<TInfoDateCubeRenewal> TInfoStrucCubeRenewal;

public:
	CPythonCubeRenewal();
	virtual ~CPythonCubeRenewal();

	bool FN_check_cube_data(CUBE_DATA* cube_data);
	bool LoadFile(const char* szFileName);
	void SetCubeRenewalHandler(PyObject* _CubeRenewalHandler) { m_CubeRenewalHandler = _CubeRenewalHandler; }

	std::vector<CUBE_DATA*> GetDataVector()
	{
		return s_cube_proto;
	}

private:
	std::vector<CUBE_DATA*>	s_cube_proto = {};
	PyObject* m_CubeRenewalHandler;
};
#endif
