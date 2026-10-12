#pragma once

#include <iostream>
#include <gtest/gtest.h>

class BaseFixture : public testing::Test {
protected:
    static void SetUpTestSuite() {
        std::cout << "Test suite started" << std::endl;
    }

    static void TearDownTestSuite() {
        std::cout << "Test suite finished" << std::endl;
    }

    void SetUp() final {
        std::cout << "Test "
                  << testing::UnitTest::GetInstance()->current_test_info()->name()
                  << " started" << std::endl;
        OnSetUp();
    }

    // Per-test setup hook for derived fixtures.
    virtual void OnSetUp() {}
};
