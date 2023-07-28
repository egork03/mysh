// Copyright (c) 2023 Egor Kosmachev
// Licensed under the MIT license.

#pragma once

#include "base/bool.h"
#include "base/plst.h"

bool glob_append(char const* glob_path, plst_t* files, plst_len_t* files_added);

