// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// Decoder
///
/// \file   dcc/rx/decoder.hpp
/// \author Vincent Hamp
/// \date   29/11/2022

#pragma once

#include "bidi.hpp"
#include "errorable.hpp"
#include "operable.hpp"
#include "readable.hpp"
#include "serviceable.hpp"
#include "writable.hpp"

namespace dcc::rx {

template<typename T>
concept Decoder = Operable<T> && Serviceable<T> && BiDi<T> && Errorable<T> &&
                  Readable<T> && Writable<T>;

} // namespace dcc::rx
