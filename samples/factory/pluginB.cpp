#include <asl/Factory.h>
#include <asl/Library.h>
#include "Animal.h"

using namespace asl;

class Cow : public Animal
{
public:
	void speak()
	{
		printf("Mooo! I am defined in a runtime loaded library\n");
	}
};

ASL_FACTORY_REGISTER(Animal, Cow)

// export so that this factory can be imported in programs loading this as at runtime:

ASL_FACTORY_EXPORT(Animal)


// export so that programs can instantiate with Library::create("Animal").

ASL_EXPORT_CLASS_AS(Cow, Animal)
