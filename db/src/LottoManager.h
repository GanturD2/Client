#include "../../common/CommonDefines.h"

#ifdef ENABLE_WORLD_LOTTERY_SYSTEM
class CLottoManager : public singleton<CLottoManager>
{
	public:
		CLottoManager();
		virtual ~CLottoManager();

		void CheckRefreshTime();
};
#endif
