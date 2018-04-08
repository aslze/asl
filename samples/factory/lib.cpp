#include <asl/Factory.h>
#include "Animal.h"

using namespace asl;

class Dog : public Animal
{
public:
	void speak()
	{
		printf("Woof! I'm defined in a linked shared library\n");
	}
};

ASL_FACTORY_REGISTER(Animal, Dog)

// this export is not needed in this case because we are linking this library
// explicitly at build time

ASL_FACTORY_EXPORT(Animal)

