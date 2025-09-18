#include "Derived.h"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <sstream>
#include <stacktrace>

#include <fmt/ranges.h>

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
    {
        OutArchive ar(ss);
        ar(NVP(node1));
    }

    auto data = ss.str();
    SPDLOG_INFO("bytes size: {}\n{}", data.size(), data);

    {
        InArchive iar(ss);
        node1.reset();
        iar(NVP(node1));
    }
    SPDLOG_INFO("node1: {}", *node1);

    node2 = node1->GetChild(0);
    SPDLOG_INFO("node2: {}", *node2);

    node3 = node2->GetChild(0);
    SPDLOG_INFO("node3: {}", *node3);
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
    test_shared_ptr();

    return 0;
}