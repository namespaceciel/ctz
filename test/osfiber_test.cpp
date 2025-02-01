#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <ctz/fiber.hpp> // NOLINT(misc-include-cleaner)
#include <iostream>
#include <memory>
#include <string>

namespace {

// A custom, small stack size for the fibers in these tests.
// Note: Stack sizes less than 16KB may cause issues on some platforms.
// See: https://github.com/google/marl/issues/201
constexpr size_t fiberStackSize = 16 * 1024;

} // namespace

TEST(OSFiberTest, OSFiber) {
    std::string str;
    auto main = ctz::OSFiber::createFiberFromCurrentThread(); // NOLINT(misc-include-cleaner)

    std::unique_ptr<ctz::OSFiber> fiberA, fiberB, fiberC;

    fiberC = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "C";
        fiberC->switchTo(fiberB.get());
    });

    fiberB = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "B";
        fiberB->switchTo(fiberA.get());
    });

    fiberA = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "A";
        fiberA->switchTo(main.get());
    });

    main->switchTo(fiberC.get());

    ASSERT_EQ(str, "CBA");
}

TEST(OSFiberTest, OSFiber2) {
    std::string str;
    auto main = ctz::OSFiber::createFiberFromCurrentThread();

    std::unique_ptr<ctz::OSFiber> fiberA, fiberB, fiberC;

    fiberC = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "C";
        fiberC->switchTo(fiberB.get());
        str += "C";
        fiberC->switchTo(fiberB.get());
    });

    fiberB = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "B";
        fiberB->switchTo(fiberA.get());
        str += "B";
        fiberB->switchTo(fiberA.get());
    });

    fiberA = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        str += "A";
        fiberA->switchTo(fiberC.get());
        str += "A";
        fiberA->switchTo(main.get());
    });

    main->switchTo(fiberC.get());

    ASSERT_EQ(str, "CBACBA");
}

TEST(OSFiberTest, StackAlignment) {
    uintptr_t address = 0;

    struct alignas(16) AlignTo16Bytes {
        uint64_t a{};
        uint64_t b{};
    };

    auto main = ctz::OSFiber::createFiberFromCurrentThread();

    std::unique_ptr<ctz::OSFiber> fiber;

    fiber = ctz::OSFiber::createFiber(fiberStackSize, [&] {
        AlignTo16Bytes stack_var;

        address = reinterpret_cast<uintptr_t>(&stack_var);

        fiber->switchTo(main.get());
    });

    main->switchTo(fiber.get());

    ASSERT_TRUE((address & 15) == 0) << "Stack variable had unaligned address: 0x" << std::hex << address;
}
