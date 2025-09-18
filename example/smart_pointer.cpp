#include <zen_serialization/archive.h>

int main() {
    using namespace zen;

    auto intval = std::make_shared<int>(42);
    auto doubleval = std::make_unique<double>(0.2);

    std::stringstream ss;
    {
        OutArchive ar(ss);
        ar(NVP(intval), NVP(doubleval));
    }

    SPDLOG_INFO("Serialized int: {}", ss.str());

    return 0;
}