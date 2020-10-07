#pragma once

#include "gmock/gmock.h"


bool waitTillDone(testing::internal::ExpectationBase& expectation, int waittime);
