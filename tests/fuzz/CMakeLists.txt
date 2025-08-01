#
#  Copyright (c) 2025, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

set(COMMON_INCLUDES
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/tests/nexus
)

set(COMMON_COMPILE_OPTIONS
    -DOPENTHREAD_FTD=1
    -DOPENTHREAD_MTD=0
    -DOPENTHREAD_RADIO=0
)

set(COMMON_LIBS
    openthread-ftd
    ot-nexus-platform
    ${OT_MBEDTLS}
    $ENV{LIB_FUZZING_ENGINE}
    ot-config
)

#----------------------------------------------------------------------------------------------------------------------

macro(ot_nexus_test name)

    # Macro to add an OpenThread nexus test.
    #
    #   Nexus test name will be `nexus_{name}`. Test source file of
    #   `test_{name}.cpp` is used. Optional extra arguments can be
    #   passed to provide additional source files.

    add_executable(${name}-fuzzer
        ${name}.cpp ${ARGN}
    )

    target_include_directories(${name}-fuzzer
    PRIVATE
        ${COMMON_INCLUDES}
    )

    target_link_libraries(${name}-fuzzer
    PRIVATE
        ${COMMON_LIBS}
    )

    target_compile_options(${name}-fuzzer
    PRIVATE
        ${COMMON_COMPILE_OPTIONS}
    )
endmacro()

#----------------------------------------------------------------------------------------------------------------------

ot_nexus_test(icmp6)
ot_nexus_test(mdns)
ot_nexus_test(radio-one-node)
