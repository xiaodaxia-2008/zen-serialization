#include <zen_serialization/archive.h>

int main()
{
    using namespace zen;

    auto intval = std::make_shared<int>(42);
    auto doubleval = std::make_unique<double>(0.2);

    std::stringstream ss;
    OutArchive oar{GeneralOutArchive<BinarySerializer>(ss)};
    oar(NVP(intval), NVP(doubleval));
    oar.Flush();
    SPDLOG_INFO("Serialized int: {}", ss.str());

    InArchive iar{GeneralInArchive<BinaryDeserializer>(ss)};
    intval.reset();
    doubleval.reset();
    iar(NVP(intval), NVP(doubleval));

    SPDLOG_INFO("Deserialized int: {}", *intval);
    SPDLOG_INFO("Deserialized double: {}", *doubleval);

    return 0;
}