#!/usr/bin/env bash

#
# The MIT License (MIT)
#
# Copyright (c) 2021  Steffen Nuessle
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# 
# This script runs clang-format on the source files currently present in the
# git staging area. To ensure that the changes are also part of the next
# commit, all the files are added once again to the staging area.
#

# Get all (A)dded or (M)odified .c or .h files in the staging area
mapfile -t FILES < <(\
    git diff \
        --name-only \
        --cached \
        --diff-filter=AM \
        -- "*.[ch]$" \
) || true

if [[ ${#FILES[@]} -ne 0 ]]; then

    # Format all found files
    clang-format -i "${FILES[@]}"

    # Ensure that the changes to these files are also added to the staging area
    git add "${FILES[@]}"
fi

exit 0
