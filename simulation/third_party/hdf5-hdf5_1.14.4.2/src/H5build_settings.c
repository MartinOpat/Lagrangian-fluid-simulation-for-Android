/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://www.hdfgroup.org/licenses.               *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "H5private.h"

H5_GCC_DIAG_OFF("larger-than=")
H5_CLANG_DIAG_OFF("overlength-strings")

/* clang-format off */
const char H5build_settings[]=
    "        SUMMARY OF THE HDF5 CONFIGURATION\n"
    "        =================================\n"
    "\n"
    "General Information:\n"
    "-------------------\n"
    "                   HDF5 Version: 1.14.4-2\n"
    "                  Configured on: Fri Apr 19 13:22:49 CEST 2024\n"
    "                  Configured by: root@martin-XPS-9320\n"
    "                    Host system: x86_64-pc-linux-android\n"
    "              Uname information: Linux martin-XPS-9320 6.5.0-1019-oem #20-Ubuntu SMP PREEMPT_DYNAMIC Mon Mar 18 17:38:55 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux\n"
    "                       Byte sex: little-endian\n"
    "             Installation point: /home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr\n"
    "\n"
    "Compiling Options:\n"
    "------------------\n"
    "                     Build Mode: production\n"
    "              Debugging Symbols: no\n"
    "                        Asserts: no\n"
    "                      Profiling: no\n"
    "             Optimization Level: high\n"
    "\n"
    "Linking Options:\n"
    "----------------\n"
    "                      Libraries: shared\n"
    "  Statically Linked Executables: \n"
    "                        LDFLAGS: --sysroot /home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot -L/home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib\n"
    "                     H5_LDFLAGS: \n"
    "                     AM_LDFLAGS:  -L/home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib\n"
    "                Extra libraries: -lz -ldl -lm \n"
    "                       Archiver: /home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ar\n"
    "                       AR_FLAGS: cr\n"
    "                         Ranlib: /home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-ranlib\n"
    "\n"
    "Languages:\n"
    "----------\n"
    "                              C: yes\n"
    "                     C Compiler: /home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android21-clang\n"
    "                       CPPFLAGS: -I/home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include\n"
    "                    H5_CPPFLAGS: -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L   -DNDEBUG -UH5_DEBUG_API -I/home/martin/Lagrangian-fluid-simulation-for-Android/simulation/third_party/hdf5-hdf5_1.14.4.2/src/H5FDsubfiling\n"
    "                    AM_CPPFLAGS:  -I/home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include\n"
    "                        C Flags: --sysroot /home/martin/Android/Sdk/ndk/25.1.8937393/toolchains/llvm/prebuilt/linux-x86_64/sysroot\n"
    "                     H5 C Flags:       \n"
    "                     AM C Flags: \n"
    "               Shared C Library: yes\n"
    "               Static C Library: no\n"
    "\n"
    "\n"
    "                        Fortran: no\n"
    "\n"
    "                            C++: no\n"
    "\n"
    "                           Java: no\n"
    "\n"
    "\n"
    "Features:\n"
    "---------\n"
    "                     Parallel HDF5: no\n"
    "  Parallel Filtered Dataset Writes: no\n"
    "                Large Parallel I/O: no\n"
    "                High-level library: yes\n"
    "Dimension scales w/ new references: no\n"
    "                  Build HDF5 Tests: yes\n"
    "                  Build HDF5 Tools: yes\n"
    "                   Build GIF Tools: no\n"
    "                      Threadsafety: no\n"
    "               Default API mapping: v114\n"
    "    With deprecated public symbols: yes\n"
    "            I/O filters (external): deflate(zlib)\n"
    "                  _Float16 support: no\n"
    "                     Map (H5M) API: no\n"
    "                        Direct VFD: no\n"
    "                        Mirror VFD: no\n"
    "                     Subfiling VFD: no\n"
    "                (Read-Only) S3 VFD: no\n"
    "              (Read-Only) HDFS VFD: no\n"
    "    Packages w/ extra debug output: none\n"
    "                       API tracing: no\n"
    "              Using memory checker: no\n"
    "            Function stack tracing: no\n"
    "                  Use file locking: best-effort\n"
    "         Strict file format checks: no\n"
    "      Optimization instrumentation: no\n"
;
/* clang-format on */

H5_GCC_DIAG_ON("larger-than=")
H5_CLANG_DIAG_OFF("overlength-strings")
