#include <asl/Factory.h>
#include "Animal.h"

using namespace asl;

class Bird : public Animal
{
public:
	void speak()
	{
		printf("Tweet! I am defined in a runtime loaded library\n");
	}
};

ASL_FACTORY_REGISTER(Animal, Bird)

ASL_FACTORY_EXPORT(Animal)
