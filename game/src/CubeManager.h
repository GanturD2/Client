
class CCubeManager : public singleton<CCubeManager>
{
	public :
		struct CUBE_VALUE
		{
			int32_t vnum;
			int32_t count;
			bool operator == (const CUBE_VALUE& b)
			{
				return (this->count == b.count) && (this->vnum == b.vnum);
			}
		};

		struct CUBE_DATA
		{
			CUBE_DATA() : set_value(0), gem_point(0) {}

			std::vector<int32_t> npc_vnum;
			std::vector<CUBE_VALUE> item;
			std::vector<CUBE_VALUE> reward;
			int percent;
			int32_t gold;
			int32_t gem_point;
			bool allow_copy;
			std::string category;
			int not_remove;
			int set_value;
		};
		
		CCubeManager();

		bool FN_check_cube_data(CUBE_DATA* cube_data);
		bool Cube_init();
		void Cube_close(LPCHARACTER ch);
		std::vector<CUBE_DATA*> GetDataVector()
		{
			return s_cube_proto;
		}
		
		void RefineCube(LPCHARACTER ch, int vnum, int multiplier, int indexImprove, std::vector<int> listOfReqItems);

	private:
		std::vector<CUBE_DATA*>	s_cube_proto = {};
};
