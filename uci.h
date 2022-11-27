//
//  uci.h
//  Chaos Chess (Hummingbird)
//
//  Created by McKinley Keys on 1/18/22.
//

#pragma once
#ifndef uci_h
#define uci_h

#include "fruit.h"
#include "definitions.h"
#include "variants.h"

namespace UCI
{

extern Variant variant;

void run();

} // namespace UCI

#endif /* uci_h */
