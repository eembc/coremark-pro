# About

CoreMark®-PRO is a comprehensive, advanced processor benchmark that works with and enhances the market-proven industry-standard EEMBC CoreMark® benchmark. While CoreMark stresses the CPU pipeline, CoreMark-PRO tests the entire processor, adding comprehensive support for multicore technology, a combination of integer and floating-point workloads, and data sets for utilizing larger memory subsystems. Together, EEMBC CoreMark and CoreMark-PRO provide a standard benchmark covering the spectrum from low-end microcontrollers to high-performance computing processors.

The EEMBC CoreMark-PRO benchmark contains five prevalent integer workloads and four popular floating-point workloads. 

The integer workloads include:

* JPEG compression
* ZIP compression
* XML parsing
* SHA-256 Secure Hash Algorithm
* A more memory-intensive version of the original CoreMark

The floating-point workloads include:

* Radix-2 Fast Fourier Transform (FFT)
* Gaussian elimination with partial pivoting derived from LINPACK
* A simple neural-net
* A greatly improved version of the Livermore loops benchmark using the following 24 FORTRAN kernels converted to C (all of these reported as a single score of the `loops.c` workload). The standard Livermore loops include:
  *   Kernel 1 -- hydro fragment
  *   Kernel 2 -- ICCG excerpt (Incomplete Cholesky Conjugate Gradient)
  *   Kernel 3 -- inner product
  *   Kernel 4 -- banded linear equations
  *   Kernel 5 -- tri-diagonal elimination, below diagonal
  *   Kernel 6 -- general linear recurrence equations
  *   Kernel 7 -- equation of state fragment
  *   Kernel 8 -- ADI integration
  *   Kernel 9 -- integrate predictors
  *   Kernel 10 -- difference predictors
  *   Kernel 11 -- first sum
  *   Kernel 12 -- first difference
  *   Kernel 13 -- 2-D PIC (Particle In Cell)
  *   Kernel 14 -- 1-D PIC (pticle In Cell)
  *   Kernel 15 -- Casual Fortran.
  *   Kernel 16 -- Monte Carlo search loop
  *   Kernel 17 -- implicit, conditional computation
  *   Kernel 18 -- 2-D explicit hydrodynamics fragment
  *   Kernel 19 -- general linear recurrence equations
  *   Kernel 20 -- Discrete ordinates transport, conditional recurrence on xx
  *   Kernel 21 -- matrix*matrix product
  *   Kernel 22 -- Planckian distribution
  *   Kernel 23 -- 2-D implicit hydrodynamics fragment
  *   Kernel 24 -- find location of first minimum in array

The CoreMark-PRO score is a weighted geometric mean of each workload, as describe on page 12 of the provided PDF document.

# Basic Overview

Build the benchmark using the `make` command and specificying a target architecture with `TARGET=`. Accomodations for custom targets and toolchains are placed in the `util/make` folder. To compile for Linux and the gcc64 toolchain, use this command:

```
% make TARGET=linux64 build
```
This will include the `util/make/linux64.mak` file, which in turn includes the `gcc64.mak` file for the toolchain. When finished, nine executables are saved in `builds/linux64/gcc64/bin` folder. These are binaries used by the test.

The command:

```
% make TARGET=linux64 XCMD='-c4' certify-all
```

...runs all of the nine tests (with four contexts), collects their output scores, and processes them through a Perl script to generate the final CoreMark-PRO score, like so:

~~~
WORKLOAD RESULTS TABLE

                                                 MultiCore SingleCore           
Workload Name                                     (iter/s)   (iter/s)    Scaling
----------------------------------------------- ---------- ---------- ----------
cjpeg-rose7-preset                                  555.56     156.25       3.56
core                                                  4.87       1.30       3.75
linear_alg-mid-100x100-sp                          1428.57     409.84       3.49
loops-all-mid-10k-sp                                 22.56       6.25       3.61
nnet_test                                            33.22      10.56       3.15
parser-125k                                          70.18      19.23       3.65
radix2-big-64k                                     1666.67     453.72       3.67
sha-test                                            588.24     172.41       3.41
zip-test                                            500.00     142.86       3.50

MARK RESULTS TABLE

Mark Name                                        MultiCore SingleCore    Scaling
----------------------------------------------- ---------- ---------- ----------
CoreMark-PRO                                      19183.84    5439.59       3.53
~~~

This will run all nine tests twice, once with one context and once with a user-defined number of contexts, in this case four, and then generate the scaling between the two configurations. Please refer to the documentation for explanations of how to change the number of contexts and workers.

# Source Code Overview

## MITH Porting

The benchmark utilizes EEMBC's Multi-Instance Test Harness, or MITH. Found in the `mith` folder, the test harness consists of high-level functions for launching the tests, and a low-level abstraction layer (in the `al` folder) for interfacing with the hardware or operating system. **The file `th_al.c` in the `al/src` folder is the only place modifications are needed to port the benchmark to new hardware.** In fact, changing any other source files invalidates the CoreMark-PRO score.

Out of the box, the MITH abstraction layer is configured to work with the POSIX `pthread` architecture on Linux, but any thread scheduling system that can be represented through the MITH abstraction layer is valid (including no threading on baremetal). The MITH harness provieds a `mith_main` function, and the actual `main` functions are provided in the workload areas. 

The example above was run from a Linux CLI, where it is possible to invoke each binary in simple succession via the Makefile and collect scores for analysis by the Perl script. Non-Linux targets (e.g., baremetal) are more complex to run, as each binary needs to be downloaded to the hardware manually and the individual results collected from a remote debugger console by retargeting the `al_printf` function. The computation for the CoreMark-PRO score is described in the included PDF documentation.

## Workloads, Kernels, and Datasets

As stated above, each workload compiles to a single binary. The workloads in the `workloads` folder contain a top-level C-file that instantiates the test harness. A workload consists of one or more benchmark kernels (stored in the `kernels` folder), and a dataset (see NOTE below). For example, the binary `loops-all-mid-10k-sp.exe` is compiled from `workloads/loops-all-mid-10k-sp`. This workload invokes the Livermore Loops kernel from `benchmarks/loops/` and configures it to use the `ref-sp/10k.c` file. This file includes parameters for constructing a 10 KB dataset, as well as the reference data results to compare against after the benchmark completes. Floating point benchmarks check for accuracy by checking a minimum number of bits that are allowed to differ (this is of greater concern in other benchmarks like EEMBC's FPMark, which stresses single- and double-precision performance). Other benchmark kernels contain just the input dataset and no reference, such as the JPEG workload.

NOTE: In CoreMark-PRO, the mapping is 1:1, each workload invokes one kernel. Other MITH-based benchmarks from EEMBC, such as AutoBench 2.0, multiple kernels are arranged in different configurations in each workload.

# Documentation

Please refer to the PDF user guide located in the `docs` folder of this repository for more details.

More info may be found at the [EEMBC CoreMark-PRO website](https://www.eembc.org/coremark-pro/).

# Run Rules
What is and is not allowed.

## Required
1. Each workload must run for at least 1000 times the minimum timer resolution. For example, on a 10 ms timer tick based system, each workload must run for at least 10 seconds.
2. To report results, the build target `certify-all` must be used or that process must be followed if `make` is not usable (e.g. via embedded debugger runs); each workload must report no errors when run with `-v1`.
3. All workloads within CoreMark-Pro must be compiled with the same flags and linked with the same flags. These must be disclosed and/or reported with any publication of CoreMark-Pro scores.

## Allowed
1. You may change the number of iterations.
2. You may change toolchain and build/load/run options.
3. You may change the implementation of porting files under mith/al sub tree.
4. You may change makefiles or using IDE projects.
5. Profile guided optimizations are allowed on base run; if used, they must be used for all workloads.

## NOT ALLOWED
1. You may not change the source file under benchmarks or workloads folders.

# Baremetal and Other Ports

The MITH hardare abstraction layer is defined in `mith/al/src`. These files contain any low-level functions needed by the benchmark. The MITH framework is used for a number of benchmarks, so not all options are relevant to or used by CoreMark-PRO.

The provided implementaiton was tested on 32- and 64-bit Linux distributions, as well as Cygwin. Since the datasets are loaded implicitly as C-structures, file I/O is not used. The only major modification likely needed for an embedded port is how `pthreads` are implemented. Choices are:

1. Provide a POSIX thread library
2. Switch to single-thread mode by using the reference `al_single.c` instead of `al_smp.c`
3. Implement the functions `al_smp.c` using the target platform's threading SDK

There's no standard flash downloader or response extractor included because every tool chain or IDE behaves differently in this regard. One easy method is to load each compiled firmware image through an IDE debugger and extract the results either by redirecting the `th_printf` function, or simply reading the IDE debugger output assuming `vsprintf` is redirected to the IDE or console via the debuggger link. The computation of the CoreMark-PRO score is described on page 12 of the provided PDF user's guide.

# Submitting Results

CoreMark-PRO results can be submitted on the web. Open a web browser and go to the [submission page](https://www.eembc.org/coremark-pro/submit.php). After registering an account you may enter a score.

# Publication Rules

1. As stated in the license, a "Commercial COREMARK-PRO License" from EEMBC is required for Licensee to disclose, reference, or publish test results generated by COREMARK-PRO in Licensee’s marketing of any of Licensee’s commercially‐available, product‐related materials, including, but not limited to product briefs, website, product brochures, product datasheets, or any white paper or article made available for public consumption. (This does not include academic research or personal use)
2. Scores must be uploaded to the EEMBC CoreMark-PRO website before being published in any capacity to ensure run rules were followed.

# Copyright and Licensing

EEMBC and CoreMark are trademarks of EEMBC. Please refer to the file LICENSE.md for the license associated with this benchmark software.
