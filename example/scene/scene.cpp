#include "Derived.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <sstream>
#include <stacktrace>

#include <fmt/ranges.h>

template <typename TOut, typename TIn>
void test_shared_ptr()
{
    std::shared_ptr<BaseNode> node1 =
        std::make_shared<DerivedNode>("Node1", std::array<float, 3>{2, 2, 2});
    std::shared_ptr<BaseNode> node2 = std::make_shared<BaseNode>("Node2");
    std::shared_ptr<BaseNode> node3 =
        std::make_shared<DerivedNode>("Node3", std::array<float, 3>{3, 3, 3});
    node1->AddChild(node2);
    node2->AddChild(node3);
    SPDLOG_INFO("node1: {}", *node1);
    SPDLOG_INFO("node2: {}", *node2);
    SPDLOG_INFO("node3: {}", *node3);

    std::stringstream ss;
    OutArchive oar{TOut{ss}};
    oar(make_nvp("scene", node1));
    oar.Flush();

    auto data = ss.str();
    if (oar.IsBinary()) {
        SPDLOG_INFO("data size: {}\n{}", data.size(), std::span(data));
    } else {
        SPDLOG_INFO("data size: {}\n{}", data.size(), data);
    }

    std::shared_ptr<BaseNode> node11, node22, node33;
    InArchive iar{TIn(ss)};
    iar(make_nvp("scene", node11));
    SPDLOG_INFO("{}", *node1);
    SPDLOG_INFO("{}", *node11);
    ZEN_EUNSURE(fmt::format("{}", *node1) == fmt::format("{}", *node11));

    node22 = node11->GetChild(0);
    ZEN_EUNSURE(fmt::format("{}", *node2) == fmt::format("{}", *node22));

    node33 = node22->GetChild(0);
    ZEN_EUNSURE(fmt::format("{}", *node3) == fmt::format("{}", *node33));
}

namespace fs = std::filesystem;
int main()
{
    std::set_terminate([] {
        auto curexp = std::current_exception();
        if (curexp) {
            try {
                spdlog::error("{}", fmt::streamed(std::stacktrace::current()));
                std::rethrow_exception(curexp);
            } catch (std::exception &e) {
                spdlog::error("{}", e.what());
            }
        }
    });

    spdlog::set_level(spdlog::level::debug);
    test_shared_ptr<JsonSerializer, JsonDeserializer>();
    test_shared_ptr<BinarySerializer, BinaryDeserializer>();

    return 0;
}