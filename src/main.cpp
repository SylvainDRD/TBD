#include <misc/utils.hpp>

int main() {
	LOG("TEST");
	LOG("TEST" << "dsqd");
	WARN("TEST");
	WARN("TEST" << "dsqd");
	ERROR("TEST");
	ERROR("TEST" << "dsqd");
	DEBUG("TEST");
	DEBUG("TEST" << "dsqd");

	return 0;
}